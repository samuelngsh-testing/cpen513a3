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
    void setNet(int net_id, const QList<int> &conn_blocks);

    //! Check check all blocks have some connection.
    bool allBlocksConnected() const;

    //! Get block count.
    int numBlocks() const {return n_blocks_;}

    //! Get net count.
    int numNets() const {return n_nets_;}

    //! Return all nets.
    const QVector<QList<int>> &nets() const {return nets_;}

    //! Return the net with the specified ID.
    const QList<int> &net(int nid) const {return nets_[nid];}

    //! Return block net connectivity records.
    const QVector<QList<int>> &allBlockNets() const {return all_block_net_ids_;}
    
    //! Return the net connectivity of a single net.
    const QList<int> &blockNets(int bid) const {return all_block_net_ids_[bid];}

  private:

    int n_blocks_=-1; //!< Number of blocks.
    int n_nets_=-1;   //!< Number of nets.

    //! List of nets where each net consists of a list of block IDs.
    QVector<QList<int>> nets_;
    //! For each block, store a list of associated net IDs.
    QVector<QList<int>> all_block_net_ids_;
  };

  /*! \brief Chip containing two partitions for the graph to be mapped onto.
   *
   * The chip on which the problem graph is to be mapped onto.
   */
  class Chip
  {
  public:
    //! Constructor taking the graph.
    Chip(const Graph &graph);

    //! Return the graph.
    const Graph &graph() {return graph_;}

    //! Return the partition that the indexed block belongs to. -1 means unassigned.
    int blockPart(int bid) const {return block_part_[bid];}

    //! Set the specified partition to the indexed block.
    void setBlockPart(int bid, int part) {block_part_[bid] = part;}

    //! Return the full block partition record vector.
    const QVector<int> &blockParts() {return block_part_;}

    /*! \brief Static function that calculates the cost from scratch.
     *
     * Static function that calculates the cost from scratch for a given graph
     * and block partition assignment vector.
     */
    static int calcCost(const Graph &graph, const QVector<int> &block_part);

    //! Non-static override.
    int calcCost() const {return calcCost(graph_, block_part_);}

    /*! \brief Return the cost delta for a prospective assignment.
     *
     * Return the cost delta if the indexed block is set to the specified 
     * partition.
     */
    static int calcCostDelta(const Graph &graph, const QVector<int> &block_part,
        int bid, int part);

    // TODO add static version for part override
    int calcCostDelta(int bid, int part) const;

    //! Return the current stored cost.
    int cost() const {return cost_;}

    //! Set the cost.
    void setCost(int cost) {cost_ = cost;}

  private:

    /*! \brief Calculate the cost of the given net.
     *
     * Block placements are overriden by the given map where the keys are block 
     * IDs to override and the partitions to pretend the block is in.
     */
    static int netCost(int nid, const Graph &graph, const QVector<int> &block_part,
        int override_bid=-1, int override_part=-1);

    //! Net cost override call.
    int netCost(int nid, int override_bid, int override_part) const;

    Graph graph_;   //!< The graph to be mapped onto this chip.
    int cost_;      //!< The cost associated with the current partition assignment.
    QVector<int> block_part_; //!< Store the partition that the i-th block is mapped to.

  };


  // TODO remove chip class since it's not relevant for this assignment
  /*! \brief Chip spatial representation of blocks and nets.
   *
   * A chip containing certain numbers of rows and columns for blocks to be
   * placed onto. Also performs the cost calculation.
   */
  /*
  class Chip
  {
  public:
    //! Constructor taking the problem file path to be read.
    Chip(const QString &f_path);

    //! Destructor.
    ~Chip();

    //! Clear all placements.
    void initEmptyPlacements();

    //! Return whether this chip has been successfully initialized.
    bool isInitialized() {return initialized;}

    //! Return nx.
    int dimX() const {return nx;}

    //! Return ny.
    int dimY() const {return ny;}

    //! Return the number of blocks.
    int numBlocks() const {return n_blocks;}

    //! Return the number of nets.
    int numNets() const {return n_nets;}

    //! Return the graph object.
    Graph *getGraph() {return graph;}

    //! Return block IDs associated with a net
    QList<int> netBlockIds(int net_id) const;

    //! Return coordinates associated with a net
    QList<QPair<int,int>> netCoords(int net_id) const;

    //! Set the cell coordinates of a block.
    void setLocBlock(const QPair<int,int> &loc, int block_id);

    //! Return the block id at the specified cell coordinates.
    int blockIdAt(int x, int y) {return grid[x][y];}

    //! Overrided function taking a pair that represents the cell coordinates.
    int blockIdAt(QPair<int,int> coord) {return grid[coord.first][coord.second];}

    //! Return the cell coordinates of the specified block as a pair.
    QPair<int,int> blockLoc(int block_id) {return block_locs[block_id];}

    //! \brief Compute the cost of the current placement.
    //!
    //! Compute the cost of the current placement from scratch. Does not update 
    //! the internal cost counter, use setCost to do that.
    int calcCost();

    //! Set the cost to the specified value.
    void setCost(int t_cost) {cost = t_cost;}

    //! Return the current stored cost of the problem without recalculating it.
    int getCost() const {return cost;}

    //! Compute the cost delta for executing a swap between two coordinates.
    //! Does not update the internal cost.
    int calcSwapCostDelta(int x1, int y1, int x2, int y2);

    //! Set the grid to the provided 2D matrix.
    void setGrid(const QVector<QVector<int>> &t_grid, bool skip_validation=false);
    
    //! Calculate and return the cost of the specified net ID.
    int costOfNet(int net_id) const;

  private:

    // Private variables
    Graph *graph=nullptr;   //!< Graph object that holds the connectivities.
    bool initialized=false; //!< Indication of whether this chip is initialized.
    int cost=-1;            //!< Current cost of the placement, -1 if no placement.
    int nx=0;               //!< Max cell count in the x direction.
    int ny=0;               //!< Max cell count in the y direction.
    int n_blocks=0;         //!< Number of blocks in the problem.
    int n_nets=0;           //!< Number of nets in the problem.
    QVector<QVector<int>> grid; //!< A grid storing the block ID associated to each cell. -1 if empty.
    QVector<QPair<int,int>> block_locs; //!< Store all block locations.

  };
  */

}

#endif
