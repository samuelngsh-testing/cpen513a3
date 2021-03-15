/*!
  \file partviewer.cc
  \author Samuel Ng
  \date 2021-03-08 created
  \copyright GNU LGPL v3
  */

#include "partviewer.h"

#define ind_coord(ind,nx) QPoint(ind%nx, (ind-ind%nx)/nx);

using namespace gui;

PartViewer::PartViewer(QWidget *parent)
  : QGraphicsView(parent), dim_x(-1), dim_y(-1)
{
  initPartViewer();
}

PartViewer::~PartViewer()
{
  clearProblem();
}

void PartViewer::showGraphPart(sp::Graph *graph, const QVector<int> &block_part)
{
  // clear existing elements first
  clearProblem();
  graph_ = graph;

  // determine grid sizes of both sides based on total block count
  int part_dim = std::ceil(std::sqrt(graph_->numBlocks()/2.));
  dim_x = part_dim*2 + 3;  // +3 is for spcers on the left, divider, and right
  dim_y = part_dim + 2;    // +2 is for top and bottom spacers
  int x_divide = part_dim+1;

  // place blocks into partitions in sequence
  block_locs_.resize(graph_->numBlocks());
  QVector<int> part_inds = {0, 0};
  for (int bid=0; bid<block_part.size(); bid++) {
    int part = block_part[bid];
    int part_ind = part_inds[part]++;
    QPoint block_loc = ind_coord(part_ind, part_dim);
    block_loc += QPoint((part==1)?x_divide+1:1, 1);
    block_locs_[bid] = block_loc;
  }

  // sanity check on the blocks
  if (std::abs(part_inds[0]-part_inds[1]) != graph_->numBlocks() % 2) {
    QMessageBox::warning(this, tr("Unexpected block counts"),
        tr("Block count in partitions differ from expectation."));
    qDebug() << "Partition assignments:" << block_part;
    qDebug() << "Partition indices:" << part_inds;
  }

  // create GUI primitives for cells
  for (int x=0; x<dim_x; x++) {
    for (int y=0; y<dim_y; y++) {
      Cell::Role role = Cell::Empty;
      if (x == 0 || x == dim_x-1 || x == x_divide ||
          y == 0 || y == dim_y-1) {
        role = Cell::Spacer;
      }
      QPoint loc(x, y);
      Cell *cell = new Cell(loc, role);
      cell_prims_.insert(qMakePair(loc.x(), loc.y()), cell);
      scene_->addItem(cell);
    }
  }

  // set the appropriate cells to occupied
  for (const QPoint &block_loc : block_locs_) {
    cell_prims_[qMakePair(block_loc.x(), block_loc.y())]->setRole(Cell::Occupied);
  }

  // create GUI primitives for nets
  net_prims_.resize(graph_->numNets());
  for (int nid=0; nid<graph_->numNets(); nid++) {
    Net *net = new Net(nid, graph_->numNets(), graph_->net(nid), block_locs_, x_divide);
    net_prims_[nid] = net;
    scene_->addItem(net);
  }

  fitProblemInView();
}

void PartViewer::clearProblem()
{
  // remove records of block locations
  block_locs_.clear();

  // remove prim display elements
  scene_->clear();
  net_prims_.clear();
  cell_prims_.clear();
}

void PartViewer::fitProblemInView()
{
  if (dim_x == -1) {
    // uninitialized, do nothing
    return;
  }
  QRectF rect(0, 0, st::Settings::sf_grid*dim_x, st::Settings::sf_grid*dim_y);
  setSceneRect(rect);
  fitInView(rect, Qt::KeepAspectRatio);
}

void PartViewer::initPartViewer()
{
  scene_ = new QGraphicsScene(this);
  setScene(scene_);
  setMinimumSize(300,300);
}
