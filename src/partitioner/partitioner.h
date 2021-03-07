/*! 
  \file partitioner.h
  \brief The branch and bound partitioning algorithm.
  \author Samuel Ng
  \date 2021-02-24 created
  \copyright GNU LGPL v3
  */

#ifndef _PT_PARTITIONER_H_
#define _PT_PARTITIONER_H_

#include <QObject>
#include <random>
#include "spatial.h"

namespace pt {

  // forward declarations
  class Partitioner;

  //! Recursively assign blocks to partitions (Steve's routine).
  // TODO parallel implementation could run this to a certain level, then split
  // children to different threads
  void assignNext(int tid, Partitioner *partitioner, const QVector<int> &curr_assignment,
      int bid, quint64 part_a_count, quint64 part_b_count, QVector<int> *best_assignment,
      int *best_cost);

  /* \brief Settings for the partitioner
   */
  struct PSettings
  {
    // runtime settings
    int threads=2;            //!< CPU threads to use, must be 2^N. TODO change to 1 at launch
    int gui_update_batch=100; //!< Update GUI each time this number of prune branches have been stored

    // pruning settings
    bool prune_half=true;     //!< Prune half of the tree (since it's mirrored)
    bool prune_by_cost=true;  //!< Prune branches that have higher cost

    // preferences
    bool no_gui=false;        //!< No GUI feedback for this problem
    bool verbose=false;       //!< Print diagnostics. TODO change to default false at launch
  };

  /*! \brief Partitioning algorithm class.
   *
   * This class contains methods that facilitate and perform branch and bound 
   * partitioning.
   */
  class Partitioner : public QThread
  {
    Q_OBJECT
  public:
    /*! \brief Contructor.
     *
     * Constructor taking the problem. TODO pointer to 
     * some data class for main GUI visualization
     */
    Partitioner(const sp::Graph &graph, const PSettings &settings=PSettings());

    //! Run the partitioner. TODO settings if any
    void runPartitioner();

    //! Inform partitioner of new pruned branches
    void newPrune(int tid, int bid, const QVector<int> &assignments);

    //! Information exchange at leaf node.
    void leafReachedExchange(int tid,  int *best_cost);

    //! Return the best cost.
    int bestCost() {return best_cost_;}

    //! Set a new best cost.
    void setBestCost(int c) {best_cost_ = c;}

    //! Return the current graph.
    const sp::Graph &graph() {return graph_;}

    //! Return the current settings.
    const PSettings &settings() {return settings_;}

    //! Return the maximum blocks allowed in partition.
    quint64 maxBlocksInPart() {return max_blocks_in_part_;}

  signals:

    //! Emitted to inform of new prunes to be visualized.
    void sig_pruned(QQueue<QPair<int,QVector<int>>> *bid_as_pairs);

    //! Emitted to show updated telemetry information.
    void sig_updateTelem(quint64 visited, quint64 pruned, int best_cut);

  private:

    //! Recursively assign blocks to partitions (Steve's routine).
    /* TODO remove
    void assignNext(const QVector<int> &curr_assignment, int bid, int part_a_count, 
        int part_b_count, QVector<int> *best_assignment, int *best_cost);
        */

    // variables
    sp::Graph graph_;         //!< Graph containing the problem.
    PSettings settings_;      //!< Partitioner settings.
    int best_cost_;           //!< Known best cost so far.
    quint64 max_blocks_in_part_;  //!< Maximum count of blocks in partition.
    quint64 visited_leaves_;     //!< Keep track of the visited node count.
    QVector<quint64> pruned_leaves_; //!< Keep track of the pruned node count.
    // TODO remove std::mutex emit_mutex;  //!< Mutex for emitting pruned branch.
    QList<QMutex*> emit_mutexes_; //!< Mutexes for emitting pruned branches.
    QMutex leaf_exch_mutex_;  //!< Mutex for leaf info exchange.
    QVector<QQueue<QPair<int,QVector<int>>>> bid_assignment_pairs_;
  };

  class PartitionerThread : public QThread
  {
    Q_OBJECT
  public:
    //! Construct a partitioner thread.
    PartitionerThread(int tid, const sp::Graph &graph, const PSettings &settings,
        const QVector<int> &curr_assignment, int start_bid,
        QVector<int> *best_assignment, int *best_cost, Partitioner *parent);

    //! Run the partitioner.
    void run() override;

  signals:

    void sig_infoExchange(int prune_bid, const QVector<int> &prune_assignment,
        int *new_best_cost);

  private:

    /*! \brief Recursively assign blocks to partitions.
     *
     * This is basically "Steve's routine" but spawns new threads at a specified
     * depth.
     */
    /* TODO remove
    void assignNext(const QVector<int> &curr_assignment, int bid, int part_a_count,
        int part_b_count, QVector<int> *best_assignment, int *best_cost);
        */


    int tid_;               //!< Thread ID.
    sp::Graph graph_;       //!< Graph containing the problem.
    PSettings settings_;    //!< Partitioner settings.
    QVector<int> init_assignment_;  //!< Initial assignments.
    int start_bid_;         //!< Starting block ID
    QVector<int> *best_assignment_; //!< Pointer to best assignment so far.
    int *best_cost_;        //!< Pointer to best cost so far.
    Partitioner *parent_;
  };

}

#endif
