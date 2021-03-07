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
    int *best_cost)
{
  // TODO current implementation recalculates cost from scratch every time. Optimize later.
  if (bid == partitioner->graph().numBlocks()) {
    // reached leaf (caller would have already assigned the leaf node partitions)
    // calc cost and update best
    int leaf_cost = sp::Chip::calcCost(partitioner->graph(), curr_assignment);
    if (partitioner->settings().verbose) {
      qDebug() << "Leaf reached with cost " << leaf_cost << curr_assignment;
    }
    if (leaf_cost < *best_cost || *best_cost < 0) {
      *best_cost = leaf_cost;
      *best_assignment = curr_assignment;
    }
    partitioner->leafReachedExchange(tid, best_cost);
    return;
  } else if (part_a_count > partitioner->maxBlocksInPart() 
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
  } else if (partitioner->settings().prune_by_cost && *best_cost >= 0 
      && sp::Chip::calcCost(partitioner->graph(), curr_assignment) > *best_cost) {
    // cut this branch if the cost is already higher than best
    if (partitioner->settings().verbose) {
      qDebug() << "Pruned costly branch at" << curr_assignment;
    }
    partitioner->newPrune(tid, bid, curr_assignment);
    return;
  } else {
    // recurse into left branch
    QVector<int> ca_a = curr_assignment;
    ca_a[bid] = 0;
    ::assignNext(tid, partitioner, ca_a, bid+1, part_a_count+1, part_b_count, 
        best_assignment, best_cost);
    // recurse into right branch
    QVector<int> ca_b = curr_assignment;
    ca_b[bid] = 1;
    ::assignNext(tid, partitioner, ca_b, bid+1, part_a_count, part_b_count+1, 
        best_assignment, best_cost);
    return;
  }
}

Partitioner::Partitioner(const sp::Graph &graph, const PSettings &settings)
  : graph_(graph), settings_(settings), best_cost_(-1), visited_leaves_(0)
{
  int numer = graph_.numBlocks();
  if (numer % 2 == 1) {
    numer++;
  }
  max_blocks_in_part_ = std::llround(numer/2.);
  qDebug() << "Block count:" << graph_.numBlocks() << ", max in partition:" << max_blocks_in_part_;
}

void Partitioner::newPrune(int tid, int bid, const QVector<int> &assignments)
{
  if (!settings_.no_gui) {
    if (tid == -1) {
      // special treatment if single threaded
      bid_assignment_pairs_[0].enqueue(qMakePair(bid, assignments));
      if (bid_assignment_pairs_[0].size() >= settings_.gui_update_batch) {
        emit sig_pruned(&bid_assignment_pairs_[0]);
      }
    } else {
      // thread-safe way to enqueue prune GUI updates, but rely on main loop to 
      // actuall emit the changes
      emit_mutexes_[tid]->lock();
      bid_assignment_pairs_[tid].enqueue(qMakePair(bid, assignments));
      emit_mutexes_[tid]->unlock();
    }
  }
  pruned_leaves_[(tid)>=0?tid:0] += std::llround(std::pow(2, graph_.numBlocks()-bid));
}

void Partitioner::leafReachedExchange(int tid, int *best_cost)
{
  leaf_exch_mutex_.lock();
  if (bestCost() < 0 || (*best_cost >= 0 && *best_cost < bestCost())) {
    setBestCost(*best_cost);
  } else if (bestCost() < *best_cost){
    *best_cost = bestCost();
  }
  visited_leaves_++;
  if (tid == -1) {
    quint64 pruned_leaves = std::accumulate(pruned_leaves_.cbegin(), pruned_leaves_.cend(), 0L);
    emit sig_updateTelem(visited_leaves_, pruned_leaves, best_cost_);
  }
  leaf_exch_mutex_.unlock();
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
  int split_at_bid = log2(actual_th); // ensure thread count is 2^x by casting to int
  actual_th = pow(2, split_at_bid);
  if (graph_.numBlocks() <= split_at_bid) {
    split_at_bid = 0;
  }

  // initialize vars
  QVector<int> curr_assignment, best_assignment;  // block assignments so far
  curr_assignment.resize(graph_.numBlocks());
  best_assignment.resize(graph_.numBlocks());
  for (int i=0; i<graph_.numBlocks(); i++) {
    curr_assignment[i] = -1;
    best_assignment[i] = -1;
  }

  // prepare pre-assignments and spawn threads
  if (actual_th == 1) {
    // simpler routine for just a single thread
    int best_cost = -1;
    pruned_leaves_.resize(1);
    bid_assignment_pairs_.resize(1);
    ::assignNext(-1, this, curr_assignment, 0, 0, 0, &best_assignment, &best_cost);
    // re-emit leftover assignments and telemetries
    if (!bid_assignment_pairs_[0].isEmpty()) {
      emit sig_pruned(&bid_assignment_pairs_[0]);
    }
    quint64 pruned_leaves = std::accumulate(pruned_leaves_.cbegin(), pruned_leaves_.cend(), 0L);
    emit sig_updateTelem(visited_leaves_, pruned_leaves, best_cost_);
    qDebug() << "best cost: " << best_cost;
  } else {
    // multi-threaded routine
    int sleep_ms = (graph_.numBlocks() >= 70) ? 1000:10;
    QList<QThread*> threads;
    QVector<int> best_costs;
    QVector<QVector<int>> best_assignments;
    QVector<bool> finished;
    best_costs.resize(actual_th);
    best_assignments.resize(actual_th);
    bid_assignment_pairs_.resize(actual_th);
    pruned_leaves_.resize(actual_th);
    finished.resize(actual_th);
    qDebug() << QObject::tr("Spawning %1 threads").arg(actual_th);
    // init curr assignment list
    for (int i=0; i<=split_at_bid; i++) {
      curr_assignment[i] = 0;
    }
    for (quint64 tid=0; tid<actual_th; tid++) {
      // pre-assignments
      bool carry=true;
      int part_a_count=0;
      int part_b_count=0;
      // increment curr assignment list
      if (tid > 0) {
        for (int j=split_at_bid; j>0; j--) {
          if (j==split_at_bid && curr_assignment[j] == 0) {
            curr_assignment[j] = 1;
            carry = false;
          } else if (carry && curr_assignment[j] == 1) {
            curr_assignment[j] = 0;
            carry = true;
          } else if (carry) {
            curr_assignment[j] = 1;
            carry = false;
          }
          if (curr_assignment[j] == 0) {
            part_a_count++;
          } else {
            part_b_count++;
          }
        }
      }
      qDebug() << curr_assignment;
      // spawn threads
      best_assignments[tid].resize(graph_.numBlocks());
      best_costs[tid] = -1;
      finished[tid] = false;
      pruned_leaves_[tid] = 0;
      emit_mutexes_.push_back(new QMutex());
      PartitionerThread *worker_th = new PartitionerThread(tid, graph_, settings_, 
          curr_assignment, split_at_bid+1, &best_assignments[tid], &best_costs[tid],
          this);
      worker_th->start();
      threads.append(worker_th);
      connect(worker_th, &QThread::finished,
          [&finished, tid](){finished[tid]=true;});
    }
    if (settings_.prune_half) {
      // prune right half
      newPrune(0, 1, {1});
      // TODO remove bid_assignment_pairs_[0].enqueue(QPair<int,QVector<int>>(1, {1}));
    }

    // wait for completion
    qDebug() << "Waiting for all instances to complete...";
    while (!std::all_of(finished.cbegin(), finished.cend(), [](bool f){return f;})) {
      msleep(sleep_ms);
      if (!settings_.no_gui) {
        for (quint64 tid=0; tid<actual_th; tid++) {
          if (bid_assignment_pairs_[tid].size() >= settings_.gui_update_batch) {
            emit_mutexes_[tid]->lock();
            emit sig_pruned(&bid_assignment_pairs_[tid]);
            emit_mutexes_[tid]->unlock();
          }
        }
      }
      leaf_exch_mutex_.lock();
      quint64 pruned_leaves = std::accumulate(pruned_leaves_.cbegin(), pruned_leaves_.cend(), 0L);
      emit sig_updateTelem(visited_leaves_, pruned_leaves, best_cost_);
      leaf_exch_mutex_.unlock();
    }
    // tidy up
    quint64 pruned_leaves = std::accumulate(pruned_leaves_.cbegin(), pruned_leaves_.cend(), 0L);
    emit sig_updateTelem(visited_leaves_, pruned_leaves, best_cost_);
    for (quint64 tid=0; tid<actual_th; tid++) {
      // emit remaining pruned branches
      if (!bid_assignment_pairs_[tid].isEmpty()) {
        emit sig_pruned(&bid_assignment_pairs_[tid]);
      }
      // clean up mutexes
      delete emit_mutexes_[tid];
      emit_mutexes_[tid] = nullptr;
    }
    emit_mutexes_.clear();

    // TODO grab the best result out of the list. Just printing whole best_costs array to term for now
    qDebug() << "Best cost list:" << best_costs;
  }
}

// thread implementation
PartitionerThread::PartitionerThread(int tid, const sp::Graph &graph, 
    const PSettings &settings, const QVector<int> &curr_assignment, 
    int start_bid, QVector<int> *best_assignment, int *best_cost, 
    Partitioner *parent)
  :  tid_(tid), graph_(graph), settings_(settings), init_assignment_(curr_assignment),
     start_bid_(start_bid), best_assignment_(best_assignment),
     best_cost_(best_cost), parent_(parent)
{
}

void PartitionerThread::run()
{
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
      part_b_count, best_assignment_, best_cost_);

}
