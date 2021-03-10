/*! 
  \file partitioner.cc
  \author Samuel Ng
  \date 2021-02-24 created
  \copyright GNU LGPL v3
  */

#include "partitioner.h"
#include <thread>

using namespace pt;

void assignNext(int tid, Partitioner *partitioner, const QVector<int> &curr_assignment,
    int bid, quint64 part_a_count, quint64 part_b_count, QVector<int> *best_assignment,
    int *local_best_cost, int global_best_cost)
{
  if (part_a_count > partitioner->maxBlocksInPart() 
      || part_b_count > partitioner->maxBlocksInPart()) {
    if (partitioner->settings().verbose) {
      qDebug() << "Pruned imbalanced branch at" << curr_assignment;
    }
    partitioner->newPrune(tid, bid, curr_assignment);
  } else if (partitioner->settings().prune_half && bid==1 && curr_assignment[0]==1) {
    // prune right half of the tree as it's just a mirror of the left half
    if (partitioner->settings().verbose) {
      qDebug() << "Pruned right half of the tree.";
    }
    partitioner->newPrune(tid, bid, curr_assignment);
    return;
  } else if (partitioner->settings().prune_by_cost && global_best_cost >= 0 
      && sp::Chip::calcCost(partitioner->graph(), curr_assignment) > global_best_cost) {
    // cut this branch if the cost is already higher than best
    if (partitioner->settings().verbose) {
      qDebug() << "Pruned costly branch at" << curr_assignment;
    }
    partitioner->newPrune(tid, bid, curr_assignment);
    return;
  } else if (bid == partitioner->graph().numBlocks()) {
    // reached leaf (caller would have already assigned the leaf node partitions)
    // calc cost and update best
    int leaf_cost = sp::Chip::calcCost(partitioner->graph(), curr_assignment);
    if (partitioner->settings().verbose) {
      qDebug() << "Leaf reached with cost " << leaf_cost << curr_assignment;
    }
    if (leaf_cost < *local_best_cost || *local_best_cost < 0) {
      *local_best_cost = leaf_cost;
      *best_assignment = curr_assignment;
    }
    partitioner->leafReachedExchange(tid, local_best_cost, global_best_cost);
    return;
  } else {
    // recurse into left branch
    QVector<int> ca_a = curr_assignment;
    ca_a[bid] = 0;
    ::assignNext(tid, partitioner, ca_a, bid+1, part_a_count+1, part_b_count, 
        best_assignment, local_best_cost, global_best_cost);
    // recurse into right branch
    QVector<int> ca_b = curr_assignment;
    ca_b[bid] = 1;
    ::assignNext(tid, partitioner, ca_b, bid+1, part_a_count, part_b_count+1, 
        best_assignment, local_best_cost, global_best_cost);
    return;
  }
}

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

  // prepare pre-assignments and spawn threads
  /* TODO remove
  if (actual_th_count_ == 0) { // TODO hack
    // simpler routine for just a single thread
    int best_cost = -1;
    visited_leaves_.resize(1);
    pruned_leaves_.resize(1);
    visited_leaves_[0] = 0;
    pruned_leaves_[0] = 0;
    bid_assignment_pairs_.resize(1);
    ::assignNext(-1, this, curr_assignment, 0, 0, 0, &best_assignment, &best_cost, -1);
    // re-emit leftover assignments and telemetries
    if (!bid_assignment_pairs_[0].isEmpty()) {
      emit sig_pruned(&bid_assignment_pairs_[0]);
    }
    emit sig_updateTelem(visited_leaves_[0], pruned_leaves_[0], best_cost_);
    qDebug() << "best cost: " << best_cost;
  } else {
  */
  // multi-threaded routine
  int sleep_ms = (graph_.numBlocks() >= 70) ? 1000:100;
  /* TODO remove
     QVector<int> best_costs;
     QVector<QVector<int>> best_assignments;
     */
  QVector<bool> finished;
  /* TODO remove
     best_costs.resize(actual_th_count_);
     best_assignments.resize(actual_th_count_);
     */
  best_costs_.resize(actual_th_count_);
  best_assignments_.resize(actual_th_count_);
  bid_assignment_pairs_.resize(actual_th_count_);
  visited_leaves_.resize(actual_th_count_);
  pruned_leaves_.resize(actual_th_count_);
  finished.resize(actual_th_count_); // TODO remove if signal end works
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
    /* TODO remove
       best_assignments[tid].resize(graph_.numBlocks());
       best_costs[tid] = -1;
       */
    best_assignments_[tid].resize(graph_.numBlocks());
    best_costs_[tid] = -1;
    finished[tid] = false;
    visited_leaves_[tid] = 0;
    PartitionerThread *worker_th = new PartitionerThread(tid, graph_, settings_, 
        curr_assignment, split_at_bid_+1, &best_assignments_[tid], &best_costs_[tid],
        this);
    worker_th->start();
    threads.append(worker_th);
    /* TODO remove
       connect(worker_th, &QThread::finished,
       [&finished, tid](){finished[tid]=true;});
       */
    connect(worker_th, &QThread::finished, this, &Partitioner::processCompletedThread);
  }

  // set up timer to update GUI periodically while the workers run
  qDebug() << "Setting up GUI update timer";
  gui_update_timer_ = new QTimer(this);
  connect(gui_update_timer_, &QTimer::timeout, [this](){sendGuiUpdates();});
  gui_update_timer_->start(sleep_ms);

  qDebug() << "Workers setup complete. Wait for completion.";

    // wait for completion
    /* TODO remove
    qDebug() << "Waiting for all instances to complete...";
    for (auto &th : threads) {
      th->wait();
    }
    */
    /* TODO remove
    while (!std::all_of(finished.cbegin(), finished.cend(), [](bool f){return f;})) {
      msleep(sleep_ms);
      if (!settings_.headless) {
        emitPrunedBranches();
        emit sig_updateTelem(visitedLeafCount(), prunedLeafCount(), best_cost_);
      }
    }
    */

    // tidy up
    /* TODO remove
    qDebug() << "Tidying up after partitioning";
    emitPrunedBranches(true);
    emit sig_updateTelem(visitedLeafCount(), prunedLeafCount(), best_cost_);

    // TODO grab the best result out of the list. Just printing whole best_costs array to term for now
    qDebug() << "Best cost list:" << best_costs;
    // emit the best assignment
    int best_cost = -1;
    for (quint64 tid=0; tid<actual_th_count_; tid++) {
      if (best_cost == -1 || best_costs[tid] >= 0) {
        best_cost = best_costs[tid];
        best_assignment = best_assignments[tid];
      }
    }
    */
  //}
  // TODO remove if finish signal works
  /* TODO remove
  if (!settings_.headless) {
    emit sig_bestPart(&graph_, best_assignment);
  }
  */
}

void Partitioner::newPrune(int tid, int bid, const QVector<int> &assignments)
{
  // TODO remove int add_pruned_leaves = std::llround(std::pow(2, graph_.numBlocks()-bid));
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
    if (!settings_.no_dtv && !settings_.headless) {
      QMutexLocker locker(prune_mutex_[tid]);
      bid_assignment_pairs_[tid].enqueue(qMakePair(bid, assignments));
    }
  }
  if (tid < 0) tid = 0;
  pruned_leaves_[tid] += std::llround(std::pow(2, graph_.numBlocks()-bid));
}

void Partitioner::leafReachedExchange(int tid, int *local_best_cost, int &global_best_cost)
{
  if (tid < 0) tid = 0;
  if (bestCost() < 0 || (*local_best_cost >= 0 && *local_best_cost < bestCost())) {
    setBestCost(*local_best_cost);
  }
  if (bestCost() < global_best_cost){
    global_best_cost = bestCost();
  }
  visited_leaves_[tid]++;
  if (tid == -1 && !settings_.headless) {
    emit sig_updateTelem(visited_leaves_[0], pruned_leaves_[0], best_cost_);
  }
}

void Partitioner::processCompletedThread()
{
  complete_mutex_.lock();

  qDebug() << "A thread has completed. Processing completion actions for it...";
  if (--remaining_th_ == 0) {
    qDebug() << "All threads have completed. Processing completion actions...";
    // all done, wrap up
    gui_update_timer_->stop();

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

    // emit the best partition
    if (!settings_.headless) {
      emit sig_bestPart(&graph_, best_assignment);
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
  /*
  quint64 part_a_count = 0;
  quint64 part_b_count = 0;
  for (int i=0; i<start_bid_; i++) {
    if (init_assignment_[i] == 0) {
      part_a_count++;
    } else if (init_assignment_[i] == 1) {
      part_b_count++;
    } else {
      qFatal("Partition thread encountered unassigned block.");
    }
  }
  ::assignNext(tid_, parent_, init_assignment_, start_bid_, part_a_count, 
      part_b_count, best_assignment_, local_best_cost_, -1);
      */

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

  // init problem
  ProblemNodeParams p_init(init_assignment_, start_bid_, init_part_a_count, 
      init_part_b_count);
  problem_stack.push(p_init);

  // for the 0-th thread, also visit right branch to prune it
  if (tid_ == 0 && settings_.prune_half) {
    ProblemNodeParams p_r(init_assignment_, 1, 0, 1);
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

    int cut_size = sp::Chip::calcCost(parent_->graph(), p.assignment);
    if (parent_->settings().prune_by_cost && global_best_cost >= 0
        && cut_size > global_best_cost) {
      // prune by cost
      if (parent_->settings().verbose) {
        qDebug() << "Pruned costly branch at" << p.assignment;
      }
      parent_->newPrune(tid_, p.bid, p.assignment);
    } else if (p.bid == parent_->graph().numBlocks()) {
      // reached leaf, calc cost and update best
      if (parent_->settings().verbose) {
        qDebug() << "Leaf reached with cost" << cut_size << p.assignment;
      }
      if (cut_size < *local_best_cost_ || *local_best_cost_ < 0) {
        *local_best_cost_ = cut_size;
        *best_assignment_ = p.assignment;
      }
      parent_->leafReachedExchange(tid_, local_best_cost_, global_best_cost);
    } else {
      ProblemNodeParams next_p_l(p.assignment, p.bid+1, p.part_a_count, p.part_b_count);
      ProblemNodeParams next_p_r(next_p_l);
      // push right branch into traversal stack
      next_p_r.assignment[p.bid] = 1;
      next_p_r.part_b_count += 1;
      problem_stack.push(next_p_r);
      // push left branch into traversal stack
      next_p_l.assignment[p.bid] = 0;
      next_p_l.part_a_count += 1;
      problem_stack.push(next_p_l);
    }

  }
}
