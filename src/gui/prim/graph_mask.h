/*!
  \file graph_mask.h
  \brief Show either the base graph or an overlay mask as solid filled blocks.
  \author Samuel Ng
  \date 2021-03-04 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_GRAPH_MASK_H_
#define _GUI_GRAPH_MASK_H_

#include <QtWidgets>
#include "gui/settings.h"

namespace gui {

  //! A helper class for finding 2D spatial points on a map.
  class GraphHelper
  {
  public:

    /*!
     * Return the count of horizontal nodes at the bottom layer counted from the
     * provided block ID.
     */
    static qreal bottomHorizontalNodes(int bid, int num_blocks);

    /*!
     * Return the inter-node width at the specified layer.
     */
    static qreal interNodeWidth(int bid, int num_blocks);

    /*!
     * Return the leftmost node offset for the provided node level.
     */
    static qreal leftmostNodeOffset(int bid, int num_blocks);

    /*!
     * Return a list of QPoints representing points associated with a graph mask
     * that starts at the specified block ID and prior assignments.
     */
    static QVector<QPointF> maskPoints(int bid, int num_blocks, 
        const QVector<int> &assignments, QRect &bounding_rect);

  };

  //! A primitive graphical element that displays a graph block.
  class GraphMask : public QGraphicsItem
  {
  public:
    enum MaskType{Explorable, Blocked};

    /*! \brief Constructor taking the top block ID and assignments.
     *
     * Constructor taking the block ID at the top of the graph, the total block 
     * count, and the series of assignments leading there.
     */
    GraphMask(MaskType type, int bid, int num_blocks, 
        const QVector<int> &assignments);

    //! Overriden method to return the proper bounding rectangle of this cell.
    virtual QRectF boundingRect() const override;

    //! Set GUI visualization state (whether this mask will be used).
    void setGuiState(bool s) {gui_state_ = s;}

    //! Overridden method to paint this graph mask on scene.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:

    MaskType type_;
    QVector<QPointF> graph_points_; //!< Points that define the outer edges of the graph.
    QRectF bounding_rect_;          //!< Store the bounding rect.
    bool gui_state_=true;           //!< Respond to no GUI options.

  };

}


#endif
