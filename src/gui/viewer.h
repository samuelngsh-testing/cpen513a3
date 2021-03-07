/*!
  \file viewer.h
  \brief Show the problem grid and the inter-cell connectivities.
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_VIEWER_H_
#define _GUI_VIEWER_H_

#include <QtWidgets>
#include "spatial.h"
#include "prim/graph_mask.h"

namespace gui {

  //! Widget for visualizing block placements and net connectivities.
  class Viewer : public QGraphicsView
  {
    Q_OBJECT

  public:

    //! Constructor.
    Viewer(QWidget *parent=nullptr);

    //! Destructor.
    ~Viewer();

    //! Instruct viewer to show the provided graph tree.
    void showGraph(sp::Graph *graph);

    //! Instruct viewer to add prune mask.
    void addPruneMask(int bid, const QVector<int> &assignments);

    //! Instruct viewer to show the provided problem.
    // TODO remove
    //void showChip(sp::Chip *t_chip);

    //! Instruct viewer to clear any existing problems.
    void clearProblem();

    //! Fit problem in viewport.
    void fitProblemInView();

    //! Set GUI state (basically gray out masks if false).
    void setGuiState(bool);

  private:

    //! Initialize the viewer's GUI elements.
    void initViewer();

    /* TODO remove
    //! Update cell states.
    void updateCells();

    //! Update nets according to the current chip state.
    void updateNets();
    */

    // Private variables
    QGraphicsScene *scene_=nullptr;   //!< Pointer to the scene object.
    sp::Graph *graph_=nullptr;        //!< Graph pointer.
    QVector<GraphMask*> masks_;       //!< Graph masks for painting.
    bool gui_state_=true;
    /* TODO remove
    sp::Chip *chip_=nullptr;          //!< Pointer to the chip currently shown.
    QVector<QVector<Cell*>> cells_;   //!< Grid of cell pointers.
    QVector<Net*> nets_;              //!< List of net pointers.
    */

  };
}

#endif
