/*!
  \file invoker.h
  \brief GUI for tweaking invocation parameters.
  \author Samuel Ng
  \date 2021-02-15
  \copyright GNU LGPL v3
  */

#ifndef _GUI_INVOKER_H_
#define _GUI_INVOKER_H_

#include <QtWidgets>
#include "partitioner/partitioner.h"

namespace gui {

  //! GUI form for tweaking placement settings and invoking it.
  class Invoker : public QWidget
  {
    Q_OBJECT

  public:
    //! Constructor.
    Invoker(QWidget *parent=nullptr);

    //! Destructor.
    ~Invoker() {};

    //! Respond to a new graph that has been loaded.
    void respondToNewGraph(const sp::Graph &graph);

    //! Invoke placement with the current GUI settings.
    void invokePlacement();

  signals:
    //! Emit SASettings for invocation.
    void sig_runPartitioner(pt::PSettings);

    //! Emit no GUI update state.
    void sig_grayOutDecisionTree(bool);

  private:

    //! Initialize the widget.
    void initGui();

    // Private variables
    QLineEdit *le_threads;
    QLineEdit *le_gui_update_batch;
    QCheckBox *cb_prune_half;
    QCheckBox *cb_prune_by_cost;
    QCheckBox *cb_no_dtv;
    QCheckBox *cb_verbose;
    QCheckBox *cb_sanity_check;
    QCheckBox *cb_no_pie;
  };


}


#endif
