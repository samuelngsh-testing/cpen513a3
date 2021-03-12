/*!
  \file benchmarker.h
  \brief Benchmarking routine.
  \author Samuel Ng
  \date 2021-03-10 created
  \copyright GNU LGPL v3
  */

#ifndef _CLI_BENCHMARKER_H_
#define _CLI_BENCHMARKER_H_

#include <QtWidgets>
#include "partitioner/partitioner.h"

namespace cli {

  //! Run benchmarks.
  class Benchmarker : public QThread
  {
  Q_OBJECT

  public:
    //! Constructor taking the output JSON path.
    Benchmarker(const QString &json_out_path, int repeat_count, 
        const QString &settings_path="");

    //! Run benchmarks.
    void run() override;

    //! Condition variable to notify main thread of completion.
    std::condition_variable cv;
    bool ready = false;
    bool done = false;
    std::mutex m;
    std::unique_lock<std::mutex> lk;

  private:

    //! Process results and start next benchmark.
    void next(pt::PResults results, bool first_one);

    //! Read and store settings if path specified.
    void readSettings(const QString &settings_path);

    //! Write-out results.
    void writeResults();

    // Private variables
    QString json_out_path_;         //!< Output path to write to.
    int repeat_count_;              //!< Repeat each benchmarkthis many times.
    QStringList bench_names_;       //!< File names for the benchmarks (excl. suffix).
    pt::PSettings settings_;        //!< Partitioner settings.
    pt::Partitioner *partitioner_;  //!< Current partitioner.
    QStack<QPair<QString, int>> b_stack_;             //!< Benchmark stack.
    QMap<QPair<QString, int>, pt::PResults> results_; //!< Store results.

  };

}

#endif
