// @file:     main.cc
// @author:   Samuel Ng
// @created:  2021-02-02
// @license:  GNU LGPL v3
//
// @desc:     Main function for the partitioner.

#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QDebug>

#include "gui/mainwindow.h"
#include "partitioner/partitioner.h"

int main(int argc, char **argv) {
  // initialize QApplication
  QApplication app(argc, argv);
  app.setApplicationName("Branch and Bound Partitioning Program");

  // specify possible command line inputs
  QCommandLineParser parser;
  parser.setApplicationDescription("Branch and bound partitioning program for "
      "CPEN 513 by Samuel Ng.");
  parser.addHelpOption();
  parser.addPositionalArgument("in_file", "Input file specifying the problem to"
      " be partitioned (optional, can be selected from the GUI).");
  parser.addOption({"headless", "Run the problem in headless mode and output "
      "the final cost as terminal output. Must provide in_file in this case."});
  parser.addOption({"verbose", "Verbose terminal outputs (only applicable to "
      "headless mode."});
  parser.process(app);

  // get input file path
  const QStringList args = parser.positionalArguments();
  QString in_path;
  if (!args.empty()) {
    in_path = args[0];
    qDebug() << QObject::tr("Input file path: %1").arg(in_path);
  }

  // headless mode
  if (parser.isSet("headless")) {
    // run the problem in headless mode and output cost in terminal
    pt::PSettings settings;
    settings.verbose = parser.isSet("verbose");
    pt::Partitioner partitioner(in_path, settings);
    partitioner.runPartitioner();
    return 0;
  }

  // show the main GUI
  gui::MainWindow mw(in_path);
  mw.show();

  // run app
  return app.exec();
}