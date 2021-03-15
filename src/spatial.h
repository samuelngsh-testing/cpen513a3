/*!
  \file spatial.h
  \brief Spatial definitions (chip and the blocks that go into it).
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#ifndef _SP_SPATIAL_H_
#define _SP_SPATIAL_H_

#include <QtWidgets>

namespace sp {

  /*! \brief Graph of blocks and nets.
   *
   * Graph-like data structure with nodes denoting blocks. This class has no 
   * knowledge about the actual spatial placement of the blocks.
   */
  class Graph
  {
  public:
    //! Constructor taking the input file path to be read.
    Graph(const QString &f_path);

    //! Constructor taking the number of blocks and nets expected.
    //Graph(int n_blocks, int n_nets);

    //! Set the connected blocks for the specified net ID.
    void setNet(int net_id, const QVector<int> &conn_blocks);

    //! Check check all blocks have some connection.
    bool allBlocksConnected() const;

    //! Get block count.
    int numBlocks() const {return n_blocks_;}

    //! Get net count.
    int numNets() const {return n_nets_;}

    //! Return all nets.
    const QVector<QVector<int>> &nets() const {return nets_;}

    //! Return the net with the specified ID.
    const QVector<int> &net(int nid) const {return nets_[nid];}

    //! Return block net connectivity records.
    const QVector<QVector<int>> &allBlockNets() const {return all_block_net_ids_;}
    
    //! Return the net connectivity of a single net.
    const QVector<int> &blockNets(int bid) const {return all_block_net_ids_[bid];}

  private:

    int n_blocks_=-1; //!< Number of blocks.
    int n_nets_=-1;   //!< Number of nets.

    //! List of nets where each net consists of a list of block IDs.
    QVector<QVector<int>> nets_;
    //! For each block, store a list of associated net IDs.
    QVector<QVector<int>> all_block_net_ids_;
  };

  /*! \brief Chip containing two partitions for the graph to be mapped onto.
   *
   * The chip on which the problem graph is to be mapped onto.
   */
  class Chip
  {
  public:
    /*! \brief Static function that calculates the cost from scratch.
     *
     * Static function that calculates the cost from scratch for a given graph
     * and block partition assignment vector.
     */
    static int calcCost(const Graph &graph, const QVector<int> &block_part);

    /*! \brief Return the cost delta for a prospective assignment.
     *
     * Return the cost delta if the indexed block is set to the specified 
     * partition.
     */
    static int calcCostDelta(const Graph &graph, const QVector<int> &block_part,
        int bid, int part, QVector<int> &curr_net_costs);

    /*! \brief Calculate the cost of the given net.
     *
     * Block placements are overriden by the given map where the keys are block 
     * IDs to override and the partitions to pretend the block is in.
     */
    static int netCost(int nid, const Graph &graph, const QVector<int> &block_part,
        int override_bid=-1, int override_part=-1);
  };

}

#endif
