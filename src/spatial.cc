/*!
  \file spatial.cc
  \brief Spatial definitions (chip and the blocks that go into it).
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#include "spatial.h"
#include <algorithm>

using namespace sp;


// Graph class implementations

Graph::Graph(const QString &f_path)
{
  QFile in_file(f_path);
  if (!in_file.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Unable to open file for reading.";
    return;
  }

  // parse the file
  int net_id = -1;
  bool exit_cond = false;
  while (!exit_cond) {
    QString line = in_file.readLine().trimmed();
    QStringList line_items = line.split(" ");

    if (net_id == -1) {
      // if the net ID is -1 then read the problem definition
      if (line_items.size() != 2) {
        qWarning("First line of the input file must contain 2 values. File invalid.");
        return;
      }
      // read the values and assign Graph pointer
      n_blocks_ = line_items[0].toInt();
      n_nets_ = line_items[1].toInt();
      all_block_net_ids_.resize(n_blocks_);
      nets_.resize(n_nets_);
    } else {
      // read net definitions and add to Graph
      int num_blocks = line_items[0].toInt();
      QList<int> conn_blocks;
      for (int it_id=1; it_id < line_items.size(); it_id++) {
        conn_blocks.append(line_items[it_id].toInt());
      }
      if (num_blocks != conn_blocks.size()) {
        qFatal("Mismatching block counts encountered while reading input file.");
      }
      setNet(net_id, conn_blocks);
    }
    net_id++;
    exit_cond = (in_file.atEnd() || net_id >= n_nets_);
  }
  if (net_id == -1) {
    qWarning() << "Nothing was read. Check input file.";
  }
  in_file.close();

  // sanity check on the produced Graph
  if (!allBlocksConnected()) {
    qWarning() << "There are blocks on the produced Graph that aren't connected"
      " to anything";
  }
}

void Graph::setNet(int net_id, const QList<int> &conn_blocks)
{
  nets_[net_id] = conn_blocks;
  for (int b_id : conn_blocks) {
    all_block_net_ids_[b_id].append(net_id);
  }
}

bool Graph::allBlocksConnected() const
{
  for (const QList<int> &block_net_ids : all_block_net_ids_) {
    if (block_net_ids.isEmpty()) {
      return false;
    }
  }
  return true;
}


Chip::Chip(const Graph &graph)
  : graph_(graph)
{
  // initialize all block partitions to -1 (i.e., unassigned)
  block_part_.resize(graph_.numBlocks());
  for (auto it=block_part_.begin(); it!=block_part_.end(); it++) *it = -1;
}

int Chip::calcCost(const Graph &graph, const QVector<int> &block_part)
{
  int calc_cost = 0;
  for (int nid=0; nid<graph.numNets(); nid++) {
    calc_cost += netCost(nid, graph, block_part);
  }
  return calc_cost;
}

int Chip::calcCostDelta(const Graph &graph, const QVector<int> &block_part,
    int bid, int part)
{
  // get the nets that associate with this block
  const QList<int> &block_nets = graph.blockNets(bid);

  // calculate the current cost of those nets
  int cost_i = 0;
  for (int nid : block_nets) {
    cost_i += netCost(nid, graph, block_part);
  }

  // calculate the new cost if bid is set to part
  int cost_f = 0;
  for (int nid : block_nets) {
    cost_f += netCost(nid, graph, block_part, bid, part);
  }

  // return the difference
  return cost_f - cost_i;
}

int Chip::calcCostDelta(int bid, int part) const
{
  return calcCostDelta(graph_, block_part_, bid, part);
}

int Chip::netCost(int nid, const Graph &graph, const QVector<int> &block_part,
    int override_bid, int override_part)
{
  // initialize tracking vars
  bool in_part_a = false;
  bool in_part_b = false;
  // iterate through all blocks in the net to see if a crossing exists
  const QList<int> &blocks = graph.net(nid);
  for (int bid : blocks) {
    // get the assigned partition or take the overriden partition
    int part = (override_bid == bid) ? override_part : block_part[bid];
    if (part == 0) {
      in_part_a = true;
    } else if (part == 1) {
      in_part_b = true;
    }
    if (in_part_a && in_part_b) break;
  }
  return (in_part_a && in_part_b) ? 1 : 0;
}

int Chip::netCost(int nid, int override_bid, int override_part) const
{
  return netCost(nid, graph_, block_part_, override_bid, override_part);
}
