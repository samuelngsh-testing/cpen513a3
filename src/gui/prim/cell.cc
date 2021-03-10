/*!
  \file cell.cc
  \author Samuel Ng
  \date 2021-03-08 created
  \copyright GNU LGPL v3
  */

#include "cell.h"

using namespace gui;

Cell::Cell(const QPoint &block_loc, Role role)
  : role_(role)
{
  scene_loc_ = block_loc * st::Settings::sf_grid;
}

QRectF Cell::boundingRect() const
{
  qreal sf = st::Settings::sf_grid;
  return QRectF(scene_loc_, scene_loc_+QPointF(sf, sf));
}

void Cell::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  QColor col;
  switch (role_) {
    case Empty:
      col = QColor("#888888");
      break;
    case Occupied:
      col = QColor("#FFFFFF");
      break;
    case Spacer:
    default:
      col = QColor("#333333");
      break;
  }
  painter->setBrush(col);
  painter->drawRect(boundingRect());
}
