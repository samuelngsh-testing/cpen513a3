// @file:     telemetrychart.cc
// @author:   Samuel Ng
// @created:  2021-02-05
// @license:  GNU LGPL v3
//
// @desc:     Implementation of TelemetryChart.

#include "telemetrychart.h"

using namespace gui;

TelemetryChart::TelemetryChart(QWidget *parent)
  : QWidget(parent)
{
  initGui();
}

void TelemetryChart::initToGraph(sp::Graph *graph)
{
  clearTelemetries();
  total_leaves_ = std::round(pow(2, graph->numBlocks()));
  l_total_leaves_->setText(QString("%1").arg(total_leaves_));
  l_unvisited_->setText(l_total_leaves_->text());
}

void TelemetryChart::updateTelemetry(quint64 visited, quint64 pruned, int best_cut)
{
  ps_visited_->setValue((qreal)visited/total_leaves_);
  ps_pruned_->setValue((qreal)pruned/total_leaves_);
  ps_unvisited_->setValue((qreal)(total_leaves_-visited-pruned)/total_leaves_);
  l_curr_best_cut_->setText(QString("%1").arg(best_cut));
  l_visited_->setText(QString("%1").arg(visited));
  l_pruned_->setText(QString("%1").arg(pruned));
  l_unvisited_->setText(QString("%1").arg(total_leaves_-visited-pruned));
}

void TelemetryChart::clearTelemetries()
{
  ps_visited_->setValue(0);
  ps_pruned_->setValue(0);
  ps_unvisited_->setValue(1);
  l_curr_best_cut_->setText("");
  l_wall_time_->setText("");
  l_total_leaves_->setText("");
  l_visited_->setText("");
  l_pruned_->setText("");
  l_unvisited_->setText("");
}

void TelemetryChart::initGui()
{
  // init pie chart
  pie_chart_ = new QChart();
  pie_series_ = new QPieSeries();
  chart_view_ = new QChartView(pie_chart_);
  chart_view_->setRenderHint(QPainter::Antialiasing);
  pie_chart_->addSeries(pie_series_);
  pie_series_->append("Visited", 0.);
  pie_series_->append("Pruned", 0.);
  pie_series_->append("Unvisited", 1.);
  ps_visited_ = pie_series_->slices().at(0);
  ps_pruned_ = pie_series_->slices().at(1);
  ps_unvisited_ = pie_series_->slices().at(2);
  ps_visited_->setBrush(Qt::green);
  ps_pruned_->setBrush(Qt::red);
  ps_unvisited_->setBrush(Qt::gray);

  // initialize status form
  l_curr_best_cut_ = new QLabel();
  l_wall_time_ = new QLabel();
  l_total_leaves_ = new QLabel();
  l_visited_ = new QLabel();
  l_pruned_ = new QLabel();
  l_unvisited_ = new QLabel();
  QFormLayout *fl_status = new QFormLayout();
  fl_status->addRow("Best cut", l_curr_best_cut_);
  fl_status->addRow("Num leaves", l_total_leaves_);
  fl_status->addRow("Visited leaves", l_visited_);
  fl_status->addRow("Pruned leaves", l_pruned_);
  fl_status->addRow("Unvisited leaves", l_unvisited_);
  fl_status->addRow("Wall time (ms)", l_wall_time_);

  // set layout
  QVBoxLayout *vb = new QVBoxLayout();
  vb->addLayout(fl_status);
  vb->addWidget(chart_view_);
  setLayout(vb);

  // initialize values
  clearTelemetries();

  // size settings
  setMinimumSize(300, 300);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}
