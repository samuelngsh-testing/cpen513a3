/*!
  \file viewer.cc
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#include "viewer.h"

using namespace gui;

Viewer::Viewer(QWidget *parent)
  : QGraphicsView(parent)
{
  initViewer();
}

Viewer::~Viewer()
{
  clearProblem();
}

void Viewer::showGraph(sp::Graph *graph)
{
  clearProblem();
  graph_ = graph;

  GraphMask *mask = new GraphMask(GraphMask::Explorable, 0, graph->numBlocks(),
      {0});
  mask->setGuiState(gui_state_);
  masks_.append(mask);
  scene_->addItem(mask);

  fitProblemInView();
}

void Viewer::addPruneMask(int bid, const QVector<int> &assignments)
{
  GraphMask *mask = new GraphMask(GraphMask::Blocked, bid, graph_->numBlocks(),
      assignments);
  masks_.append(mask);
  scene_->addItem(mask);
}

/* TODO remove
void Viewer::showChip(sp::Chip *t_chip)
{
  if (!t_chip->isInitialized()) {
    qDebug() << "Provided chip is not initialized, aborting.";
    return;
  }
  // create GUI objects if new chip
  if (chip_ != t_chip) {
    // clear the viewer
    clearProblem();

    // create graphical objects from scratch
    chip_ = t_chip;
    cells_.resize(chip_->dimX());
    for (int x=0; x<chip_->dimX(); x++) {
      cells_[x].resize(chip_->dimY());
      for (int y=0; y<chip_->dimY(); y++) {
        Cell *cell = new Cell(x, y, chip->blockIdAt(x, y));
        cells_[x][y] = cell;
        scene_->addItem(cell);
      }
    }
  } else {
    updateCells();
  }

  // update nets if there exists any
  if (chip_->numNets() > 0) {
    updateNets();
  }

  fitProblemInView();
}
*/

void Viewer::clearProblem()
{
  // clear GUI objects
  scene_->clear();

  // clear relevant vars
  masks_.clear();
  graph_ = nullptr;
}

void Viewer::fitProblemInView()
{
  if (graph_ != nullptr) {
    int nb = graph_->numBlocks();
    QRectF rect(0, 0, st::Settings::sf*pow(2, nb+1), st::Settings::sf*(nb+1));
    setSceneRect(rect);
    fitInView(rect);
  }
}

void Viewer::setGuiState(bool gs)
{
  gui_state_=gs;
  for (auto mask : masks_) {
    mask->setGuiState(gs);
    mask->update();
  }
  repaint();
}

void Viewer::initViewer()
{
  scene_ = new QGraphicsScene(this);
  setScene(scene_);
  setMinimumSize(300,300);
}

/* TODO remove
void Viewer::updateCells()
{
  // update the graphical block ID at each location
  for (int x=0; x<chip_->dimX(); x++) {
    for (int y=0; y<chip_->dimY(); y++) {
      cells[x][y]->setBlockId(chip_->blockIdAt(x,y));
    }
  }
}

void Viewer::updateNets()
{
  // if net list is empty, recreate from scratch
  if (nets_.isEmpty()) {
    nets_.resize(chip->numNets());
    for (int net_id=0; net_id<nets_.size(); net_id++) {
      nets_[net_id] = new Net(net_id, nets_.size(), chip_->dimX(), chip_->dimY(), 
          chip->netCoords(net_id));
      scene_->addItem(nets_[net_id]);
    }
  } else {
    for (int net_id=0; net_id<nets_.size(); net_id++) {
      nets_[net_id]->updateCoords(chip_->netCoords(net_id));
    }
  }
}
*/
