/*! 
  \file partitioner.cc
  \author Samuel Ng
  \date 2021-02-24 created
  \copyright GNU LGPL v3
  */

#include "partitioner.h"
#include <thread>
#include <math.h>

using namespace pt;

#define fast_2_pow(expo) ((expo==0) ? 1LL : 1LL << ((quint64)expo))

Partitioner::Partitioner(const sp::Graph &graph, const PSettings &settings)
  : graph_(graph), settings_(settings), best_cost_(-1)
{
  // set maximum block count in each partition
  int numer = graph_.numBlocks();
  if (numer % 2 == 1) {
    numer++;
  }
  max_blocks_in_part_ = std::llround(numer/2.);

  // set extra flags for headless mode
  if (settings_.headless) {
    settings_.no_dtv = true;
  }

  // status
  if (settings_.verbose) {
    qDebug() << "Block count:" << graph_.numBlocks() << ", max in partition:" 
      << max_blocks_in_part_;
  }
}

Partitioner::~Partitioner()
{
  if (!threads.empty()) {
    for (auto &th : threads) {
      th->quit();
    }
  }
}

void Partitioner::runPartitioner()
{
  // start wall timer
  wall_timer_.start();

  // determine threads to spawn
  quint64 actual_th = std::min(
      {
        (quint64)settings_.threads, 
        (quint64)std::llround(pow(2, graph_.numBlocks()-2)),
        (quint64)std::thread::hardware_concurrency()
      });
  split_at_bid_ = log2(actual_th); // ensure thread count is 2^x by casting to int
  actual_th_count_ = pow(2, split_at_bid_);
  if (graph_.numBlocks() <= split_at_bid_) {
    split_at_bid_ = 0;
  }

  // initialize vars
  QVector<int> curr_assignment, best_assignment;
  curr_assignment.resize(graph_.numBlocks());
  best_assignment.resize(graph_.numBlocks());
  for (int i=0; i<curr_assignment.size(); i++) {
    curr_assignment[i] = -1;
    best_assignment[i] = -1;
  }

  // multi-threaded routine
  int sleep_ms = (graph_.numBlocks() >= 70) ? 1000:100;
  QVector<bool> finished;
  best_costs_.resize(actual_th_count_);
  best_assignments_.resize(actual_th_count_);
  bid_assignment_pairs_.resize(actual_th_count_);
  visited_leaves_.resize(actual_th_count_);
  pruned_leaves_.resize(actual_th_count_);
  finished.resize(actual_th_count_);
  remaining_th_ = actual_th_count_;
  prune_mutex_.clear();
  qDebug() << QObject::tr("Spawning %1 threads").arg(actual_th_count_);
  // init curr assignment list
  for (int i=0; i<=split_at_bid_; i++) {
    curr_assignment[i] = 0;
  }
  for (quint64 tid=0; tid<actual_th_count_; tid++) {
    // pre-assignments
    bool carry=true;
    // increment curr assignment list
    if (tid > 0) {
      for (int j=split_at_bid_; j>0; j--) {
        if (j==split_at_bid_ && curr_assignment[j] == 0) {
          curr_assignment[j] = 1;
          carry = false;
        } else if (carry && curr_assignment[j] == 1) {
          curr_assignment[j] = 0;
          carry = true;
        } else if (carry) {
          curr_assignment[j] = 1;
          carry = false;
        }
      }
    }
    qDebug() << curr_assignment;
    // spawn threads
    prune_mutex_.append(new QMutex());
    best_assignments_[tid].resize(graph_.numBlocks());
    best_costs_[tid] = -1;
    finished[tid] = false;
    visited_leaves_[tid] = 0;
    PartitionerThread *worker_th = new PartitionerThread(tid, graph_, settings_, 
        curr_assignment, split_at_bid_+1, &best_assignments_[tid], &best_costs_[tid],
        this);
    worker_th->start();
    threads.append(worker_th);
    if (!settings_.headless) {
      connect(worker_th, &QThread::finished, this, &Partitioner::processCompletedThread);
    }
  }

  // set up timer to update GUI periodically while the workers run
  if (!settings_.headless) {
    qDebug() << "Setting up GUI update timer";
    gui_update_timer_ = new QTimer(this);
    connect(gui_update_timer_, &QTimer::timeout, [this](){sendGuiUpdates();});
    gui_update_timer_->start(sleep_ms);
  }

  qDebug() << "Workers setup complete. Wait for completion.";

  if (settings_.headless) {
    for (auto &th : threads) {
      th->wait();
    }
    qDebug() << "Headless partitioning complete.";
    processCompletedThread();
  }
}

void Partitioner::newPrune(int tid, int bid, const QVector<int> &assignments)
{
  if (tid == -1) {
    // special treatment if single threaded
    if (!settings_.no_dtv && !settings_.headless) {
      bid_assignment_pairs_[0].enqueue(qMakePair(bid, assignments));
    }
    if (bid_assignment_pairs_[0].size() >= settings_.gui_update_batch) {
      emit sig_pruned(&bid_assignment_pairs_[0]);
    }
  } else {
    // thread-safe way to enqueue prune GUI updates, but rely on main loop to 
    // actually emit the changes
    if (!(settings_.no_dtv || settings_.headless)) {
      QMutexLocker locker(prune_mutex_[tid]);
      bid_assignment_pairs_[tid].enqueue(qMakePair(bid, assignments));
    }
  }
  if (!settings_.no_pie) {
    if (tid < 0) tid = 0;
    pruned_leaves_[tid] += std::llround(fast_2_pow(graph_.numBlocks()-bid));
  }
}

void Partitioner::leafReachedExchange(int tid, int *local_best_cost, int &global_best_cost)
{
  if (tid < 0) tid = 0;
  if (bestCost() < 0 || (*local_best_cost >= 0 && *local_best_cost < bestCost())) {
    setBestCost(*local_best_cost);
  }
  if (global_best_cost < 0 || bestCost() < global_best_cost){
    global_best_cost = bestCost();
  }
  visited_leaves_[tid]++;
  if (tid == -1 && !settings_.headless) {
    emit sig_updateTelem(visited_leaves_[0], pruned_leaves_[0], best_cost_);
  }
}

void Partitioner::processCompletedThread()
{
  qDebug() << "A thread has completed. Processing completion actions for it...";

  complete_mutex_.lock();

  if (--remaining_th_ == 0) {
    // all done, wrap up
    qDebug() << "All threads have completed. Processing completion actions...";
    if (!settings_.headless) {
      gui_update_timer_->stop();
    }
    qint64 elapsed_time = wall_timer_.elapsed();

    qDebug() << "Tidying up after partitioning";
    sendGuiUpdates(true);

    qDebug() << "Best cost list:" << best_costs_;
    // find the best assignment
    int best_cost = -1;
    QVector<int> best_assignment;
    for (quint64 tid=0; tid<actual_th_count_; tid++) {
      if (best_cost == -1 || best_costs_[tid] >= 0) {
        best_cost = best_costs_[tid];
        best_assignment = best_assignments_[tid];
      }
    }

    if (!settings_.headless) {
      // emit the best partition
      emit sig_bestPart(&graph_, best_assignment, elapsed_time);
    } else {
      // emit the result package
      PResults results;
      results.best_cut_size = best_cost;
      results.wall_time = elapsed_time;
      emit sig_packagedResults(results);
    }
  }

  complete_mutex_.unlock();
}

void Partitioner::sendGuiUpdates(bool emit_all)
{
  if (!settings_.headless) {
    emitPrunedBranches(emit_all);
    emit sig_updateTelem(visitedLeafCount(), prunedLeafCount(), best_cost_);
  }
}

void Partitioner::emitPrunedBranches(bool emit_all)
{
  if (!settings_.no_dtv && !settings_.headless) {
    for (quint64 tid=0; tid<actual_th_count_; tid++) {
      if (emit_all || bid_assignment_pairs_[tid].size() >= settings_.gui_update_batch) {
        // copy the prune queue then emit
        // sacrificing memory efficiency for shorter lock time
        prune_mutex_[tid]->lock();
        auto queue_copy = bid_assignment_pairs_[tid];
        bid_assignment_pairs_[tid].clear();
        prune_mutex_[tid]->unlock();
        emit sig_pruned(&queue_copy);
      }
    }
  }
}

// thread implementation
PartitionerThread::PartitionerThread(int tid, const sp::Graph &graph, 
    PSettings settings, QVector<int> curr_assignment, 
    int start_bid, QVector<int> *best_assignment, int *local_best_cost, 
    Partitioner *parent)
  :  tid_(tid), graph_(graph), settings_(settings), init_assignment_(curr_assignment),
     start_bid_(start_bid), best_assignment_(best_assignment),
     local_best_cost_(local_best_cost), parent_(parent)
{
}

void PartitionerThread::run()
{
  traverseProblemSpace();
}

void PartitionerThread::traverseProblemSpace()
{
  QStack<ProblemNodeParams> problem_stack;

  // init
  quint64 init_part_a_count = 0;
  quint64 init_part_b_count = 0;
  for (int i=0; i<start_bid_; i++) {
    if (init_assignment_[i] == 0) {
      init_part_a_count++;
    } else if (init_assignment_[i] == 1) {
      init_part_b_count++;
    } else {
      qFatal("Partition thread encountered unassigned block.");
    }
  }

  // tracking vars
  int global_best_cost = -1;
  QVector<int> net_costs(graph_.numNets(), -1);

  // init problem
  ProblemNodeParams p_init(init_assignment_, start_bid_, init_part_a_count, 
      init_part_b_count, net_costs);
  problem_stack.push(p_init);

  // for the 0-th thread, also visit right branch to prune it
  if (tid_ == 0 && settings_.prune_half) {
    ProblemNodeParams p_r(init_assignment_, 1, 0, 1, net_costs);
    p_r.assignment[0] = 1;
    problem_stack.push(p_r);
  }

  // traverse
  while (!problem_stack.isEmpty()) {
    ProblemNodeParams p = problem_stack.pop();
    if (p.part_a_count > parent_->maxBlocksInPart()
        || p.part_b_count > parent_->maxBlocksInPart()) {
      // prune imbalance branch
      if (parent_->settings().verbose) {
        qDebug() << "Pruned imbalance branch at" << p.assignment;
      }
      parent_->newPrune(tid_, p.bid, p.assignment);
      continue;
    } else if (parent_->settings().prune_half && p.bid==1 && p.assignment[0]==1) {
      // prune right half of the tree as it's just a mirror of the left half
      if (parent_->settings().verbose) {
        qDebug() << "Pruned right half of the tree.";
      }
      parent_->newPrune(tid_, p.bid, p.assignment);
      continue;
    }

    if (p.cut_size < 0) {
      p.cut_size = sp::Chip::calcCost(parent_->graph(), p.assignment);
    } else if (settings_.sanity_check) {
      int true_cut_size = sp::Chip::calcCost(parent_->graph(), p.assignment);
      if (p.cut_size != true_cut_size) {
        qWarning() << QString("Delta cut-size %1 is different from calculated "
            "cut size %2").arg(p.cut_size).arg(true_cut_size) << p.assignment;
        p.cut_size = sp::Chip::calcCost(parent_->graph(), p.assignment);
      }
    }
    if (p.bid != parent_->graph().numBlocks() && parent_->settings().prune_by_cost 
        && global_best_cost >= 0 && p.cut_size > global_best_cost) {
      // prune by cost
      if (parent_->settings().verbose) {
        qDebug() << "Pruned costly branch at" << p.assignment;
      }
      parent_->newPrune(tid_, p.bid, p.assignment);
    } else if (p.bid == parent_->graph().numBlocks()) {
      // reached leaf, calc cost and update best
      if (parent_->settings().verbose) {
        qDebug() << "Leaf reached with cost" << p.cut_size << p.assignment;
      }
      if (p.cut_size < *local_best_cost_ || *local_best_cost_ < 0) {
        *local_best_cost_ = p.cut_size;
        *best_assignment_ = p.assignment;
      }
      parent_->leafReachedExchange(tid_, local_best_cost_, global_best_cost);
    } else {
      // calculate next cut sizes
      int cut_size_r = p.cut_size + sp::Chip::calcCostDelta(graph_, p.assignment, p.bid, 1, p.net_costs);
      int cut_size_l = p.cut_size + sp::Chip::calcCostDelta(graph_, p.assignment, p.bid, 0, p.net_costs);
      int next_bid = p.bid;
      // repurpose p as next left branch, make a copy for right branch
      ++p.bid;
      ProblemNodeParams next_p_r(p);
      // push right branch into traversal stack
      next_p_r.cut_size = cut_size_r;
      next_p_r.assignment[next_bid] = 1;
      ++next_p_r.part_b_count;
      problem_stack.push(next_p_r);
      // push left branch
      p.cut_size = cut_size_l;
      p.assignment[next_bid] = 0;
      ++p.part_a_count;
      problem_stack.push(p);
    }

  }
}

PartitionerBusyWrapper::PartitionerBusyWrapper(const sp::Graph &graph,
    PSettings settings)
{
  settings.headless = true;
  settings.no_dtv = true;
  settings.no_pie = true;
  p = new Partitioner(graph, settings);
}

PResults PartitionerBusyWrapper::runPartitioner()
{
  PResults results;
  connect(p, &Partitioner::sig_packagedResults,
      [&results](const PResults &t_results) {
        results = t_results;
        qDebug() << "Received results from partitioner";
      });

  p->runPartitioner();

  return results;
}
