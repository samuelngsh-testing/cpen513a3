/*!
  \file dtviewer.cc
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#include "dtviewer.h"

using namespace gui;

DTViewer::DTViewer(QWidget *parent)
  : QGraphicsView(parent)
{
  initDTViewer();
}

DTViewer::~DTViewer()
{
  clearProblem();
}

void DTViewer::showGraph(sp::Graph *graph)
{
  clearProblem();
  graph_ = graph;

  GraphMask *mask = new GraphMask(GraphMask::Explorable, 0, graph->numBlocks(),
      {0});
  mask->setGrayOut(gray_state_);
  masks_.append(mask);
  scene_->addItem(mask);

  fitProblemInView();
}

void DTViewer::addPruneMask(int bid, const QVector<int> &assignments)
{
  GraphMask *mask = new GraphMask(GraphMask::Blocked, bid, graph_->numBlocks(),
      assignments);
  masks_.append(mask);
  scene_->addItem(mask);
}

void DTViewer::clearProblem()
{
  // clear GUI objects
  scene_->clear();

  // clear relevant vars
  masks_.clear();
  graph_ = nullptr;
}

void DTViewer::fitProblemInView()
{
  if (graph_ != nullptr) {
    int nb = graph_->numBlocks();
    QRectF rect(0, 0, st::Settings::sf*pow(2, nb+1), st::Settings::sf*(nb+1));
    setSceneRect(rect);
    fitInView(rect);
  }
}

void DTViewer::setGrayOut(bool gs)
{
  gray_state_=gs;
  for (auto mask : masks_) {
    mask->setGrayOut(gs);
    mask->update();
  }
  repaint();
}

void DTViewer::initDTViewer()
{
  scene_ = new QGraphicsScene(this);
  setScene(scene_);
  setMinimumSize(300,300);
}
