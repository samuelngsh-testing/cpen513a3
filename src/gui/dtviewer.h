/*!
  \file dtviewer.h
  \brief Show the decision tree.
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_DTVIEWER_H_
#define _GUI_DTVIEWER_H_

#include <QtWidgets>
#include "spatial.h"
#include "prim/graph_mask.h"

namespace gui {

  //! Widget for visualizing the decision tree.
  class DTViewer : public QGraphicsView
  {
    Q_OBJECT

  public:

    //! Constructor.
    DTViewer(QWidget *parent=nullptr);

    //! Destructor.
    ~DTViewer();

    //! Instruct viewer to show the provided graph tree.
    void showGraph(sp::Graph *graph);

    //! Instruct viewer to add prune mask.
    void addPruneMask(int bid, const QVector<int> &assignments);

    //! Instruct viewer to clear any existing problems.
    void clearProblem();

    //! Fit problem in viewport.
    void fitProblemInView();

    //! Set gray out state (basically gray out masks if true).
    void setGrayOut(bool);

  private:

    //! Initialize the viewer's GUI elements.
    void initDTViewer();

    // Private variables
    QGraphicsScene *scene_=nullptr;   //!< Pointer to the scene object.
    sp::Graph *graph_=nullptr;        //!< Graph pointer.
    QVector<GraphMask*> masks_;       //!< Graph masks for painting.
    bool gray_state_=false;
  };
}

#endif
