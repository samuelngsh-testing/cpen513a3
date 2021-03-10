/*!
  \file net.cc
  \author Samuel Ng
  \date 2021-03-08 created
  \copyright GNU LGPL v3
  */

#include <QRandomGenerator>
#include "net.h"

using namespace gui;

Net::Net(int nid, int num_nets, const QList<int> &bids,
    const QVector<QPoint> &block_locs, int x_divide)
  : nid_(nid), num_nets_(num_nets), bids_(bids)
{
  if (bids_.size() < 2) {
    qWarning() << "There should be 2+ blocks in a net. Fewer detected in net" << nid;
  }
  setBlockLocs(block_locs, x_divide);
}

void Net::setBlockLocs(const QVector<QPoint> &block_locs, int x_divide)
{
  bids_a_.clear();
  bids_b_.clear();
  locs_.clear();
  for (int bid : bids_) {
    const QPoint &loc = block_locs[bid];
    locs_.insert(bid, loc);
    if (loc.x() < x_divide) {
      bids_a_.append(bid);
    } else {
      bids_b_.append(bid);
    }
  }
  pickLeaders();
}

QRectF Net::boundingRect() const
{
  qreal sf = st::Settings::sf_grid;
  QList<QPoint> locs = locs_.values();
  QRectF br(locs[0]*sf, locs[1]*sf);
  for (int i=2; i<locs.size(); i++) {
    const QPointF &pt = locs[i]*sf;
    if (pt.x() > br.right()) {
      br.setRight(pt.x());
    }
    if (pt.x() < br.left()) {
      br.setLeft(pt.x());
    }
    if (pt.y() > br.bottom()) {
      br.setBottom(pt.y());
    }
    if (pt.y() < br.top()) {
      br.setTop(pt.y());
    }
  }
  return br;
}

void Net::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  qreal sf = st::Settings::sf_grid;
  QPointF offset(sf/2, sf/2);

  // get color
  painter->setPen(st::Settings::colorGenerator(nid_, num_nets_));

  // lambda function for drawing lines for the specified partition params
  auto draw_lines_in_part = [this,sf,offset,&painter](const QList<int> &bids, int leader)
  {
    for (int bid : bids) {
      if (bid == leader) {
        continue;
      }
      const QPointF &pt = locs_[bid]*sf;
      painter->drawLine(locs_[leader]*sf+offset, pt+offset);
    }
  };

  // draw lines in partition a
  if (bids_a_.size() > 0) {
    draw_lines_in_part(bids_a_, leader_a_);
  }

  // draw lines in partition b
  if (bids_b_.size() > 0) {
    draw_lines_in_part(bids_b_, leader_b_);
  }

  // draw line connecting both leaders
  if (leader_a_ >= 0 && leader_b_ >= 0) {
    painter->drawLine(locs_[leader_a_]*sf+offset, locs_[leader_b_]*sf+offset);
  }
}

void Net::pickLeaders()
{
  if (bids_a_.size() > 0 && bids_b_.size() > 0) {
    // pick a random block from each side as leader
    QRandomGenerator rng = QRandomGenerator::securelySeeded();
    leader_a_ = bids_a_[rng.bounded(bids_a_.size())];
    leader_b_ = bids_b_[rng.bounded(bids_b_.size())];
  } else if (bids_a_.size() > 0) {
    // all blocks are in a
    leader_a_ = bids_[0];
    leader_b_ = -1;
  } else {
    // all blocks are in b
    leader_a_ = -1;
    leader_b_ = bids_[0];
  }
}
