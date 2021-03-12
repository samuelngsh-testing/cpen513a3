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
#include "benchmarker.h"

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
  /* TODO remove
  parser.addOption({"headless", "Run the problem in headless mode and output "
      "the final cost as terminal output. Must provide in_file in this case."});
  parser.addOption({"threads", "Specify the number of threads to run in headless"
      " mode.", "n"});
  parser.addOption({"verbose", "Verbose terminal outputs (only applicable to "
      "headless mode."});
      */
  parser.addOption({"benchmark", "Run all benchmarks"});
  parser.addOption({"bench_settings_in", "JSON input file for benchmark settings",
      "path"});
  parser.addOption({"json_out", "Write generated data into <path>. Simply"
      " writes to out.json if unspecified.", "path"});
  parser.addOption({"repeat", "Repeat each benchmark for the specified number "
      "of times. Defaults to 5 if unspecified.", "repeat"});
  parser.process(app);

  // get input file path
  const QStringList args = parser.positionalArguments();
  QString in_path;
  if (!args.empty()) {
    in_path = args[0];
    qDebug() << QObject::tr("Input file path: %1").arg(in_path);
  }

  // benchmark mode
  if (parser.isSet("benchmark")) {
    // run the benchmarks
    QString out_name = parser.isSet("json_out") ? parser.value("json_out") : "out.json";
    QString set_name = parser.isSet("bench_settings_in") ? 
      parser.value("bench_settings_in") : "";
    int repeat = parser.isSet("repeat") ? parser.value("repeat").toInt() : 5;
    cli::Benchmarker bm(out_name, repeat, set_name);
    bm.start();

    // wait for the benchmarks to finish
    {
      qDebug() << "Wait for benchmarks to finish...";
      std::unique_lock<std::mutex> lk(bm.m);
      bm.cv.wait(lk, [&bm]{return bm.ready;});
    }
    qDebug() << "All benchmarks done.";

    // no need to show GUI
    return 0;
  }

  // headless mode
  /* TODO remove, doesn't work anymore
  if (parser.isSet("headless")) {
    // run the problem in headless mode and output cost in terminal
    pt::PSettings settings;
    settings.headless = true;
    settings.verbose = parser.isSet("verbose");
    if (parser.isSet("threads")) {
      int n_th = parser.value("threads").toInt();
      qDebug() << QString("Running %1 threads").arg(n_th);
      settings.threads = n_th;
    }
    pt::Partitioner partitioner(in_path, settings);
    partitioner.runPartitioner();
    return 0;
  }
  */

  // show the main GUI
  gui::MainWindow mw(in_path);
  mw.show();

  // run app
  return app.exec();
}
