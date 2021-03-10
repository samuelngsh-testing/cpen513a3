/*!
  \file cell.h
  \brief Show a cell that goes on the visualization grid.
  \author Samuel Ng
  \date 2021-03-08 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_BLOCK_H_
#define _GUI_BLOCK_H_

#include <QtWidgets>
#include "gui/settings.h"

namespace gui {

  //! Display a block in the appropriate location.
  class Cell : public QGraphicsItem
  {
  public:
    enum Role{Spacer, Empty, Occupied};

    //! Construct a block.
    Cell(const QPoint &block_loc, Role role);

    //! Update the cell's role.
    void setRole(Role role) {role_ = role;}

    //! Return the bounding rect of this cell.
    virtual QRectF boundingRect() const override;

    //! Paint the cell..
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:

    QPointF scene_loc_;   //!< Location of the cell on the scene.
    Role role_;

  };


}

#endif
