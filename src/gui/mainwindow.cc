/*!
  \file mainwindow.cc
  \author Samuel Ng
  \date 2021-02-02 created
  \copyright GNU LGPL v3
  */

#include "mainwindow.h"

using namespace gui;

MainWindow::MainWindow(const QString &in_path, QWidget *parent)
  : QMainWindow(parent)
{
  initGui();
  if (!in_path.isEmpty()) {
    // show the problem if the input path is not empty
    readAndShowProblem(in_path);
  }
}

MainWindow::~MainWindow()
{
  delete graph_;
  graph_ = nullptr;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
  dt_viewer_->fitProblemInView();
  p_viewer_->fitProblemInView();
  QMainWindow::resizeEvent(e);
}

void MainWindow::readAndShowProblem(const QString &in_path)
{
  setWindowTitle(tr("%1 - %2").arg(QCoreApplication::applicationName())
      .arg(QFileInfo(in_path).fileName()));

  // read the problem onto the class chip pointer
  graph_ = new sp::Graph(in_path);

  // show the problem
  dt_viewer_->showGraph(graph_);
  p_viewer_->clearProblem();
  tchart_->initToGraph(graph_);
  invoker_->respondToNewGraph(*graph_);
  dw_invoker_->raise();

  // TODO if size greater than 30, default to no GUI update

  /* TODO remove
  tchart_->clearTelemetries();
  */
}

void MainWindow::runPartitioner(const pt::PSettings &p_settings)
{
  if (graph_ == nullptr) {
    qWarning() << "runPlacement invoked when no Chip is present. Aborting.";
    QMessageBox::warning(this, "No Problem Present", "An attempt to run "
        "placement with no loaded problem has been halted.");
    return;
  }
  dt_viewer_->showGraph(graph_); // redraw the graph
  tchart_->initToGraph(graph_);
  dw_tchart_->raise();
  qDebug() << "Dispatching partition job.";
  if (partitioner != nullptr) {
    delete partitioner;
  }
  partitioner = new pt::Partitioner(*graph_, p_settings);

  // connect signals
  if (!p_settings.no_dtv) {
    connect(partitioner, &pt::Partitioner::sig_pruned, 
        [this](QQueue<QPair<int,QVector<int>>> *bid_as_pairs)
        {
          while (!bid_as_pairs->isEmpty()) {
            const QPair<int,QVector<int>> &bid_as_pair = bid_as_pairs->dequeue();
            dt_viewer_->addPruneMask(bid_as_pair.first, bid_as_pair.second);
          }
          qApp->processEvents();
        });
  }
  connect(partitioner, &pt::Partitioner::sig_updateTelem,
      [this](long visited, long pruned, int best_cut)
      {
        tchart_->updateTelemetry(visited, pruned, best_cut);
        qApp->processEvents();
        });
  connect(partitioner, &pt::Partitioner::sig_bestPart,
      p_viewer_, &PartViewer::showGraphPart);

  // run the placement
  partitioner->runPartitioner();
}

void MainWindow::initGui()
{
  // init GUI elements
  dt_viewer_ = new DTViewer(this);
  p_viewer_ = new PartViewer(this);
  invoker_ = new Invoker(this);
  tchart_ = new TelemetryChart(this);

  // signals
  connect(invoker_, &Invoker::sig_runPartitioner, this, &MainWindow::runPartitioner);
  connect(invoker_, &Invoker::sig_grayOutDecisionTree, 
      [this](bool no_dtv) {dt_viewer_->setGrayOut(no_dtv);});

  // layouts
  QVBoxLayout *vbl = new QVBoxLayout(); // main layout
  vbl->addWidget(dt_viewer_);
  vbl->addWidget(p_viewer_);

  QWidget *w_main = new QWidget(this);  // main widget that holds everything
  w_main->setLayout(vbl);
  setCentralWidget(w_main);

  // set dock widgets
  dw_invoker_ = new QDockWidget("Placement Invocation", this);
  dw_invoker_->setWidget(invoker_);
  addDockWidget(Qt::RightDockWidgetArea, dw_invoker_);
  dw_tchart_ = new QDockWidget("Placement Telemetry", this);
  dw_tchart_->setWidget(tchart_);
  addDockWidget(Qt::RightDockWidgetArea, dw_tchart_);
  tabifyDockWidget(dw_invoker_, dw_tchart_);
  dw_invoker_->raise();

  // initiate menu bars
  initMenuBar();
}

void MainWindow::initMenuBar()
{
  // initialize menus
  QMenu *file = menuBar()->addMenu(tr("&File"));
  QMenu *view = menuBar()->addMenu(tr("&View"));

  // file menu actions
  QAction *open_file = new QAction(tr("&Open..."), this);
  QAction *run_part = new QAction(tr("&Run partitioner"), this);
  QAction *quit = new QAction(tr("&Quit"), this);

  // assign keyboard shortcuts
  open_file->setShortcut(tr("CTRL+O"));
  quit->setShortcut(tr("CTRL+Q"));

  // connection action signals
  connect(open_file, &QAction::triggered, this, &MainWindow::loadProblemFromFileDialog);
  connect(run_part, &QAction::triggered, [this](){runPartitioner();});
  connect(quit, &QAction::triggered, this, &QWidget::close);

  // add actions to the appropriate menus
  file->addAction(open_file);
  file->addAction(run_part);
  file->addSeparator();
  file->addAction(quit);
  view->addAction(dw_invoker_->toggleViewAction());
  /*
  view->addAction(dw_tchart_->toggleViewAction());
  */
}

void MainWindow::loadProblemFromFileDialog()
{
  // open file dialog and load the specified file path
  QFileDialog fd;
  fd.setDefaultSuffix("txt");
  QString open_path = fd.getOpenFileName(this, tr("Open File"),
      "", tr("Text Files (*.txt);;All files (*.*)"));
  if (!open_path.isNull()) {
    readAndShowProblem(open_path);
  }
}
