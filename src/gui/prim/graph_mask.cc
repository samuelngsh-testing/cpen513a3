/*!
  \file graph_mask.cc
  \author Samuel Ng
  \date 2021-03-04 created
  \copyright GNU LGPL v3
  */

#include "graph_mask.h"

using namespace gui;

// GraphHelper implementation

qreal GraphHelper::bottomHorizontalNodes(int bid, int num_blocks)
{
  return pow(2., num_blocks-bid);
}

qreal GraphHelper::interNodeWidth(int bid, int num_blocks)
{
  return bottomHorizontalNodes(0, num_blocks) / pow(2., bid);
}

qreal GraphHelper::leftmostNodeOffset(int bid, int num_blocks)
{
  return interNodeWidth(bid, num_blocks)/2.;
}

QVector<QPointF> GraphHelper::maskPoints(int bid, int num_blocks,
    const QVector<int> &assignments, QRect &bounding_rect)
{
  qreal full_graph_width = bottomHorizontalNodes(0, num_blocks+1);
  QVector<QPointF> mask_points;
  mask_points.resize((num_blocks+1-bid)*2-1);
  qreal top_x_offset = full_graph_width/2.;
  qreal lay_x_offset = 0;
  for (int i=0; i<num_blocks+1; i++) {
    qreal x_offset = interNodeWidth(i, num_blocks+1) / 2.;
    if (i <= bid) {
      if (i != 0) {
        x_offset *= (assignments[i-1]==1) ? 1 : -1;
        top_x_offset += x_offset;
      }
      if (i == bid) {
        mask_points[0] = QPointF(top_x_offset, bid);
      }
    } else {
      lay_x_offset += x_offset;
      mask_points[i-bid] = QPointF(top_x_offset+lay_x_offset, i);
      mask_points[mask_points.size()-(i-bid)] = QPointF(top_x_offset-lay_x_offset, i);
      if (i == num_blocks) {
        bounding_rect.setRect(top_x_offset - lay_x_offset, bid, top_x_offset + lay_x_offset, i);
      }
    }
  }
  return mask_points;
}


// GraphMask implementation

GraphMask::GraphMask(MaskType type, int bid, int num_blocks,
    const QVector<int> &assignments)
  : type_(type)
{
  // initialize
  qreal sf = st::Settings::sf;
  QRect bounding_rect;
  const QVector<QPointF> &graph_pts_unscaled = GraphHelper::maskPoints(bid, 
      num_blocks, assignments, bounding_rect);
  graph_points_.resize(graph_pts_unscaled.size());
  // copy points over to local graph but multiple each with the scaling factor
  for (int i=0; i<graph_points_.size(); i++) {
    // TODO remove graph_points_[i] = (graph_pts_unscaled[i] - bounding_rect.topLeft()) * sf;
    graph_points_[i] = graph_pts_unscaled[i] * sf;
  }
  // update position and dims
  // TODO remove setPos(bounding_rect.topLeft()*sf);
  // TODO remove bounding_rect_.setRect(0, 0, bounding_rect.width()*sf, bounding_rect.height()*sf);
  setPos(0, 0);
  bounding_rect_ = QRectF(bounding_rect.topLeft()*sf, bounding_rect.bottomRight()*sf);
}

QRectF GraphMask::boundingRect() const
{
  return bounding_rect_;
}

void GraphMask::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget*)
{
  QColor col;
  if (gray_out_) {
    col = QColor("#AAAAAA");
  } else if (type_ == Explorable) {
    col = QColor("#00FF00");
  } else {
    col = QColor("#FF0000");
  }
  painter->setPen(Qt::NoPen);
  painter->setBrush(col);
  painter->drawPolygon(graph_points_);
}
