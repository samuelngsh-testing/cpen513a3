/*!
  \file net.h
  \brief Show a placed net that consists of a number of blocks.
  \author Samuel Ng
  \date 2021-03-08 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_NET_H_
#define _GUI_NET_H_

#include <QtWidgets>
#include "gui/settings.h"

namespace gui {

  //! Display nets for the partition visualization.
  class Net : public QGraphicsItem
  {
  public:
    //! Construct a net that contains the specified block IDs and provided locs.
    Net(int nid, int num_nets, const QList<int> &bids,
        const QVector<QPoint> &block_locs, int x_divide);

    //! Update the block locations with a new full block list.
    void setBlockLocs(const QVector<QPoint> &block_locs, int x_divide);

    //! Return the bounding rect of this net.
    virtual QRectF boundingRect() const override;

    //! Overriden method to paint the net connections.
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

  private:

    //! Pick leaders for both partitions.
    void pickLeaders();

    int nid_;               //!< The Net ID.
    int num_nets_;          //!< Total net count.
    QList<int> bids_;       //!< All block IDs in this net.
    QList<int> bids_a_;     //!< Block IDs in partition a.
    QList<int> bids_b_;     //!< Block IDs in partition b.
    int leader_a_;          //!< Leader block in partition a.
    int leader_b_;          //!< Leader block in partition b.
    QMap<int,QPoint> locs_; //!< Block locations.
  };

}

#endif
