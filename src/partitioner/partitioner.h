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
#include <condition_variable>
#include "spatial.h"

namespace pt {

  // forward declarations
  class Partitioner;

  //! Recursively assign blocks to partitions (Steve's routine).
  // TODO parallel implementation could run this to a certain level, then split
  // children to different threads
  void assignNext(int tid, Partitioner *partitioner, const QVector<int> &curr_assignment,
      int bid, quint64 part_a_count, quint64 part_b_count, QVector<int> *best_assignment,
      int *local_best_cost, int global_best_cost);

  /* \brief Settings for the partitioner
   */
  struct PSettings
  {
    // runtime settings
    int threads=1;            //!< CPU threads to use, must be 2^N. TODO change to 1 at launch
    int gui_update_batch=100; //!< Update GUI each time this number of prune branches have been stored

    // pruning settings
    bool prune_half=true;     //!< Prune half of the tree (since it's mirrored)
    bool prune_by_cost=true;  //!< Prune branches that have higher cost

    // preferences
    bool no_dtv=false;        //!< No decision tree view
    bool headless=false;      //!< Running in headless mode
    bool verbose=false;       //!< Print diagnostics. TODO change to default false at launch
  };

  /*! \brief Partitioning algorithm class.
   *
   * This class contains methods that facilitate and perform branch and bound 
   * partitioning.
   */
  class Partitioner : public QObject
  {
    Q_OBJECT
  public:
    /*! \brief Contructor.
     *
     * Constructor taking the problem. TODO pointer to 
     * some data class for main GUI visualization
     */
    Partitioner(const sp::Graph &graph, const PSettings &settings=PSettings());

    //! Destructor.
    ~Partitioner();

    //! Run the partitioner. TODO settings if any
    void runPartitioner();

    //! Inform partitioner of new pruned branches
    void newPrune(int tid, int bid, const QVector<int> &assignments);

    //! Information exchange at leaf node.
    void leafReachedExchange(int tid, int *local_best_cost, int &global_best_cost);

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

    //! Signal to inform of new prunes to be visualized.
    void sig_pruned(QQueue<QPair<int,QVector<int>>> *bid_as_pairs);

    //! Signal to show updated telemetry information.
    void sig_updateTelem(quint64 visited, quint64 pruned, int best_cut);

    //! Emit the best partition.
    void sig_bestPart(sp::Graph *graph, const QVector<int> block_part);

  private:

    //! Process completed threads.
    void processCompletedThread();

    //! Send GUI updates.
    void sendGuiUpdates(bool emit_all=false);

    //! Emit pruned branches for GUI update. Doesn't do so if no_gui is set in settings.
    void emitPrunedBranches(bool emit_all=false);

    //! Return the accumulated pruned leaf count.
    quint64 prunedLeafCount() {return std::accumulate(pruned_leaves_.cbegin(), pruned_leaves_.cend(), 0L);}

    //! Return the accumulated visited leaf count.
    quint64 visitedLeafCount() {return std::accumulate(visited_leaves_.cbegin(), visited_leaves_.cend(), 0L);}

    // variables
    sp::Graph graph_;         //!< Graph containing the problem.
    PSettings settings_;      //!< Partitioner settings.
    int best_cost_;           //!< Known best cost so far.
    QVector<int> best_costs_; //!< Best costs from all threads.
    QVector<QVector<int>> best_assignments_;  //!< Best asssignments from all threads.
    quint64 max_blocks_in_part_;  //!< Maximum count of blocks in partition.
    QVector<quint64> visited_leaves_; //!< Keep track of the visited node count.
    QVector<quint64> pruned_leaves_;  //!< Keep track of the pruned node count.
    QVector<QQueue<QPair<int,QVector<int>>>> bid_assignment_pairs_;

    // multi-threaded programming
    quint64 actual_th_count_; //!< Count of actual threads spawned.
    int split_at_bid_;
    QList<QThread*> threads;
    int remaining_th_;
    QVector<QMutex*> prune_mutex_;
    // TODO remove QMutex prune_mutex_;
    QMutex complete_mutex_;
    QTimer *gui_update_timer_;
  };

  //! Parameters for a node in the decision tree.
  struct ProblemNodeParams
  {
    //! Empty constructor.
    ProblemNodeParams() {};

    //! Construct with provided values.
    ProblemNodeParams(const QVector<int> &assignment, int bid, 
        quint64 part_a_count, quint64 part_b_count)
      : assignment(assignment), bid(bid), part_a_count(part_a_count),
        part_b_count(part_b_count) {};

    QVector<int> assignment;
    int bid;
    quint64 part_a_count;
    quint64 part_b_count;
  };

  class PartitionerThread : public QThread
  {
    Q_OBJECT
  public:
    //! Construct a partitioner thread.
    PartitionerThread(int tid, const sp::Graph &graph, PSettings settings,
        QVector<int> curr_assignment, int start_bid, QVector<int> *best_assignment,
        int *local_best_cost, Partitioner *parent);

    //! Run the partitioner.
    void run() override;

    //! Traverse through the binary tree by stack.
    void traverseProblemSpace();

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
    int *local_best_cost_;        //!< Pointer to best cost so far.
    Partitioner *parent_;
  };

}

#endif
