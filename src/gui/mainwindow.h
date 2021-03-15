/*!
  \file mainwindow.h
  \brief Main window (frame) of the application.
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#ifndef _GUI_MAINWINDOW_H_
#define _GUI_MAINWINDOW_H_

#include <QtWidgets>
#include "spatial.h"
#include "dtviewer.h"
#include "partviewer.h"
#include "invoker.h"
#include "telemetrychart.h"
#include "partitioner/partitioner.h"

namespace gui {

  //! Main window widget housing all GUI elements.
  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:

    //! Constructor taking the input file path describing the placement problem.
    explicit MainWindow(const QString &in_path, QWidget *parent=nullptr);

    //! Describtor.
    ~MainWindow();

    //! Override the resize event to fit the placement problem in the viewport
    //! when user resizes the main window.
    virtual void resizeEvent(QResizeEvent *) override;

    //! Read a problem file and show it in the viewer.
    void readAndShowProblem(const QString &in_path);

    //! Run placement on the current problem.
    void runPartitioner(const pt::PSettings &p_settings={});

  private:

    //! Initialize the GUI.
    void initGui();

    //! Initialize the top menu bar.
    void initMenuBar();

    //! Load problem from file dialog.
    void loadProblemFromFileDialog();

    // Private variables
    sp::Graph *graph_=nullptr;      //!< Pointer to the current graph.
    DTViewer *dt_viewer_=nullptr;   //!< Pointer to the binary tree viewer.
    PartViewer *p_viewer_=nullptr;  //!< Partition viewer.
    Invoker *invoker_=nullptr;      //!< Pointer to the Invoker widget.
    QDockWidget *dw_invoker_=nullptr; //!< Dockwidget for the invoker.
    TelemetryChart *tchart_=nullptr;  //!< Pointer to the telemetry chart.
    QDockWidget *dw_tchart_=nullptr;  //!< Dockwidget for telemetry chart.

    pt::Partitioner *partitioner=nullptr; //!< Partitioner currently in use.

  };

}

#endif
