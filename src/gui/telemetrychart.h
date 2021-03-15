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

    //! Set elapsed time
    void setElapsedTime(qint64 elapsed_time) {l_wall_time_->setText(QString("%1").arg(elapsed_time));}

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
    QLabel *l_wall_time_;     //!< Wall time.
  };

}

#endif
