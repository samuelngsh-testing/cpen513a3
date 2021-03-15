/*!
  \file partviewer.h
  \brief Show the partitioning.
  \author Samuel Ng
  \date 2021-03-08 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_PARTVIEWER_H_
#define _GUI_PARTVIEWER_H_

#include <QtWidgets>
#include "spatial.h"
#include "prim/net.h"
#include "prim/cell.h"

namespace gui {

  //! Widget for visualizing the decision tree.
  class PartViewer : public QGraphicsView
  {
    Q_OBJECT

  public:

    //! Constructor.
    PartViewer(QWidget *parent=nullptr);

    //! Destructor.
    ~PartViewer();

    //! Instruct viewer to show the provided graph tree mapped to the partition list.
    void showGraphPart(sp::Graph *graph, const QVector<int> &block_part);

    //! Instruct viewer to clear any existing problems.
    void clearProblem();

    //! Fit problem in viewport.
    void fitProblemInView();

  private:

    //! Initialize the viewer's GUI elements.
    void initPartViewer();

    // Private variables
    int dim_x, dim_y;
    QGraphicsScene *scene_=nullptr;   //!< Pointer to the scene object.
    sp::Graph *graph_=nullptr;        //!< Graph pointer.
    QVector<QPoint> block_locs_;      //!< Vector of block locations.
    QVector<Net*> net_prims_;         //!< Vector of net primitive pointers.
    QMap<QPair<int,int>,Cell*> cell_prims_; //!< Vector of block primitive pointers.

  };

}

#endif
