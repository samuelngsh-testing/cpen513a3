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

/* TODO remove
void TelemetryChart::addTelemetry(int cost, float T, float p_accept, int rw_dim)
{
  int x_step = std::max(cost_series->count(), T_series->count());
  // update values
  if (cost >= 0) {
    cost_series->append(x_step, cost);
    max_cost = std::max(max_cost, cost);
    l_curr_cost->setText(QString("%1").arg(cost));
  }
  if (T >= 0) {
    T_series->append(x_step, T);
    max_T = std::max(max_T, T);
    l_curr_T->setText(QString("%1").arg(T));
  }
  if (p_accept >= 0) {
    p_accept_series->append(x_step, p_accept);
    max_p_accept = std::max(max_p_accept, p_accept);
  }
  if (rw_dim >= 0) {
    rw_series->append(x_step, rw_dim);
    max_rw_dim = std::max(max_rw_dim, rw_dim);
  }
  // update ticks and ranges
  axis_x->setRange(0, cost_series->count()-1);
  axis_x_rw->setRange(0, p_accept_series->count()-1);
  axis_y_cost->setRange(0, max_cost);
  axis_y_T->setRange(0, max_T);
  axis_y_pa->setRange(0, max_p_accept);
  axis_y_rw->setRange(0, max_rw_dim);
  axis_y_cost->applyNiceNumbers();
  axis_y_T->applyNiceNumbers();
  axis_y_pa->applyNiceNumbers();
  axis_y_rw->applyNiceNumbers();
}
*/

void TelemetryChart::clearTelemetries()
{
  ps_visited_->setValue(0);
  ps_pruned_->setValue(0);
  ps_unvisited_->setValue(1);
  l_curr_best_cut_->setText("");
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
