// @file:     benchmarker.cc
// @author:   Samuel Ng
// @created:  2021-03-10
// @license:  GNU LGPL v3
//
// @desc:     Implementation of the benchmarking routines.

#include <QJsonDocument>
#include <QJsonObject>
#include "benchmarker.h"

using namespace cli;



Benchmarker::Benchmarker(const QString &json_out_path, int repeat_count,
    const QString &settings_path)
  : json_out_path_(json_out_path), repeat_count_(repeat_count), partitioner_(nullptr)
{
  if (!settings_path.isEmpty()) {
    readSettings(settings_path);
  }
  // twocm.txt is omitted due to problem size
  /*
  bench_names_ << "cc" << "cm82a" << "cm138a" << "cm150a" << "cm162a" << "con1"
    << "ugly8" << "ugly16" << "z4ml";
    */
  bench_names_ << "cm82a";
}

void Benchmarker::run()
{
  // try to open file for writing before actually running the benchmark, avoid 
  // write failure after running a bunch of benches.
  QFile f_out(json_out_path_);
  if (!f_out.open(QIODevice::WriteOnly)) {
    qDebug() << "Failed to open " << f_out << " for writing.";
    exit(1);
  } else {
    f_out.close();
  }

  for (const QString &bench_name : bench_names_) {
    for (int i=0; i<repeat_count_; i++) {
      b_stack_.append({bench_name, i});
    }
  }

  // TODO rmove m.lock();
  while (!b_stack_.empty()) {
    next(pt::PResults(), true);
  }
}

void Benchmarker::next(pt::PResults results, bool first_one)
{
  if (!first_one) {
    qDebug() << "Reading benchmark results";
    QPair<QString, int> name_id_pair = b_stack_.top();
    QString bench_name = name_id_pair.first;
    int bench_id = name_id_pair.second;
    results_.insert(qMakePair(bench_name, bench_id), results);
    b_stack_.pop();
  }

  if (b_stack_.size() != 0) {
    // run next benchmark in stack
    QString bench_name = b_stack_.top().first;
    qDebug() << "Run benchmark" << bench_name << "ID" << b_stack_.top().second;
    if (partitioner_ != nullptr) {
      delete partitioner_;
    }
    sp::Graph graph(":/benchmarks/" + bench_name + ".txt");
    partitioner_ = new pt::Partitioner(graph, settings_);
    partitioner_->runPartitioner();
  } else {
    // export the recorded statistics to the JSON output path
    writeResults();
    /* TODO remove 
    done = true;
    m.unlock();
    cv.notify_one();
    */
  }
}

void Benchmarker::readSettings(const QString &settings_path)
{
  qDebug() << "Reading benchmark settings from" << settings_path;

  // always set headless (unless overriden by actual file read)
  settings_.headless = true;
  settings_.no_dtv = false;
  settings_.verbose = false;
  settings_.sanity_check = false;

  // try to open file for reading
  QFile in_file(settings_path);
  if (!in_file.open(QIODevice::ReadOnly)) {
    qFatal("Unable to read benchmark settings file.");
  }

  // read text stream and create JSON doc
  QTextStream f_text(&in_file);
  QByteArray json_bytes = f_text.readAll().toLocal8Bit();
  QJsonDocument json_doc = QJsonDocument::fromJson(json_bytes);
  in_file.close();

  // check for read errors
  if (json_doc.isNull()) {
    qFatal("Failed to create JSON doc");
  }
  if (!json_doc.isObject()) {
    qFatal("JSON is not an object in unit test.");
  }

  QJsonObject json_obj = json_doc.object();
  if (json_obj.isEmpty()) {
    qFatal("JSON object is empty");
  }

  // iterate through all key value pairs and make appropriate settings
  for (auto json_it=json_obj.constBegin(); json_it!=json_obj.constEnd(); json_it++) {
    if (json_it.key() == "threads") {
      settings_.threads = json_it.value().toInt();
    } else if (json_it.key() == "prune_half") {
      settings_.prune_half = json_it.value().toBool();
    } else if (json_it.key() == "prune_by_cost") {
      settings_.prune_by_cost = json_it.value().toBool();
    } else if (json_it.key() == "verbose") {
      settings_.verbose = json_it.value().toBool();
    } else if (json_it.key() == "sanity_check") {
      settings_.sanity_check = json_it.value().toBool();
    } else {
      qWarning() << "Setting" << json_it.key() << "not implemented for benchmarking.";
    }
  }
}

void Benchmarker::writeResults()
{
  QVariantMap result_map;
  for (const QString &bench_name : bench_names_) {
    QList<QVariant> cut_sizes;
    QList<QVariant> visited_leaves;
    QList<QVariant> pruned_leaves;
    QList<QVariant> wall_times;
    for (int i=0; i<repeat_count_; i++) {
      pt::PResults r = results_.value({bench_name, i});
      cut_sizes.append(r.best_cut_size);
      visited_leaves.append(r.visited_leaves);
      pruned_leaves.append(r.pruned_leaves);
      wall_times.append(r.wall_time);
    }
    QVariantMap bench_map;
    bench_map["cut_sizes"] = cut_sizes;
    bench_map["visited_leaves"] = visited_leaves;
    bench_map["pruned_leaves"] = pruned_leaves;
    bench_map["wall_times"] = wall_times;
    result_map.insert(bench_name, bench_map);
  }

  QFile f_out(json_out_path_);
  QJsonObject json_obj = QJsonObject::fromVariantMap(result_map);
  QJsonDocument json_doc(json_obj);
  QString json_str = json_doc.toJson();
  f_out.write(json_str.toLocal8Bit());
  f_out.close();
  qDebug() << "Results written to " << json_out_path_;
}
