/*!
  \file telemetrychart.h
  \brief Widget that charts current placement telemetries.
  \author Samuel Ng
  \date 2021-02-05
  \copyright GNU LGPL v3
  */

#ifndef _GUI_TELEMCHART_H_
#define _GUI_TELEMCHART_H_

#include <QtWidgets>
#include <QtCharts>
#include "spatial.h"

namespace gui {

  using namespace QtCharts;

  //! A widget that plots telemetry relevant to the problem.
  class TelemetryChart : public QWidget
  {
    Q_OBJECT

  public:
    //! Constructor.
    TelemetryChart(QWidget *parent=nullptr);

    //! Set new problem baseline values.
    void initToGraph(sp::Graph *graph);

    //! Update visit/pruned values
    void updateTelemetry(quint64 visited, quint64 pruned, int best_cut);

    //! Clear telemetries.
    void clearTelemetries();

  private:

    //! Initialize the widget.
    void initGui();

    // baseline vars
    quint64 total_leaves_;        //!< Total number of leaves.

    // chart related vars
    QChartView *chart_view_;  //!< Qt Chart view containing the chart.
    QChart *pie_chart_;       //!< Chart that contains the pie series.
    QPieSeries *pie_series_;  //!< Pie series showing the visited, pruned, and unvisited nodes
    QPieSlice *ps_visited_;   //!< Pie slice for visited nodes
    QPieSlice *ps_pruned_;    //!< Pie slice for pruned nodes
    QPieSlice *ps_unvisited_; //!< Pie slice for unvisited nodes

    // other telemetry labels
    QLabel *l_total_leaves_;  //!< Totaly leaf count.
    QLabel *l_visited_;       //!< Visited leaf count.
    QLabel *l_pruned_;        //!< Pruned leaf count.
    QLabel *l_unvisited_;     //!< Unvisited leaf count.
    QLabel *l_curr_best_cut_; //!< Current best cut size.

    // TODO remove old vars below
    /*
    // Private variables
    QChart *chart;            //!< Chart to draw on.
    QChartView *chart_view;   //!< Qt Chart view containing the chart.
    QChart *rw_chart;         //!< Range window chart.
    QChartView *rw_view;      //!< Chart showing range window related parameters.
    QLineSeries *cost_series; //!< Line series storing the cost.
    QLineSeries *T_series;    //!< Line series storing the temperature.
    QLineSeries *p_accept_series; //!< Acceptance probability series.
    QLineSeries *rw_series;       //!< Range window dimensions series.
    QValueAxis *axis_x;       //!< x axis pointer.
    QValueAxis *axis_x_rw;    //!< x axis pointer.
    QValueAxis *axis_y_cost;  //!< y axis pointer for cost.
    QValueAxis *axis_y_T;     //!< y axis pointer for temperature.
    QValueAxis *axis_y_pa;    //!< Acceptance probability axis.
    QValueAxis *axis_y_rw;    //!< Range window dimension axis.
    QLabel *l_curr_T;         //!< Label of current temperature.
    QLabel *l_curr_cost;      //!< Label of current cost.
    float y_max_buf=1.1;      //!< Percentage buffer to add at the top of y axes.
    int max_cost=-1;          //!< Maximum cost seen.
    float max_T=-1;           //!< Maximum temperature seen.
    float max_p_accept=-1;    //!< Maximum acceptance probability seen.
    int max_rw_dim=-1;        //!< Maximum range window dimension seen.
    */
  };

}

#endif
