// @file:     invoker.cc
// @author:   Samuel Ng
// @created:  2021-02-15
// @license:  GNU LGPL v3
//
// @desc:     Net class implementation

#include "invoker.h"

using namespace gui;

Invoker::Invoker(QWidget *parent)
  : QWidget(parent)
{
  initGui();
}

void Invoker::respondToNewGraph(const sp::Graph &graph)
{
  if (graph.numBlocks() > 30) {
    cb_no_dtv->setChecked(true);
  } else {
    cb_no_dtv->setChecked(false);
  }
}

void Invoker::invokePlacement()
{
  // take settings from GUI options
  pt::PSettings p_set;
  p_set.threads = le_threads->text().toInt();
  p_set.gui_update_batch = le_gui_update_batch->text().toInt();
  p_set.prune_half = cb_prune_half->isChecked();
  p_set.prune_by_cost = cb_prune_by_cost->isChecked();
  p_set.no_dtv = cb_no_dtv->isChecked();
  p_set.no_pie = cb_no_pie->isChecked();
  p_set.verbose = cb_verbose->isChecked();
  p_set.sanity_check = cb_sanity_check->isChecked();

  emit sig_runPartitioner(p_set);
}

void Invoker::initGui()
{
  // get an instance of SASettings with default settings
  pt::PSettings p_set;

  // gui elements
  le_threads = new QLineEdit;
  le_threads->setText(QString("%1").arg(p_set.threads));

  le_gui_update_batch = new QLineEdit;
  le_gui_update_batch->setText(QString("%1").arg(p_set.gui_update_batch));

  cb_prune_half = new QCheckBox;
  cb_prune_half->setChecked(p_set.prune_half);

  cb_prune_by_cost = new QCheckBox;
  cb_prune_by_cost->setChecked(p_set.prune_by_cost);

  cb_no_dtv = new QCheckBox;
  cb_no_dtv->setChecked(p_set.no_dtv);

  cb_no_pie = new QCheckBox;
  cb_no_pie->setChecked(p_set.no_pie);

  cb_verbose = new QCheckBox;
  cb_verbose->setChecked(p_set.verbose);

  cb_sanity_check = new QCheckBox;
  cb_sanity_check->setChecked(p_set.sanity_check);

  QPushButton *pb_run_partitioner = new QPushButton("Run");
  pb_run_partitioner->setShortcut(tr("CTRL+R"));

  // init gui elements
  connect(pb_run_partitioner, &QAbstractButton::released, this, &Invoker::invokePlacement);
  connect(cb_no_dtv, &QCheckBox::stateChanged,
      [this](int state) {emit sig_grayOutDecisionTree(state);});

  // add items to layout
  QFormLayout *fl_gen = new QFormLayout();
  fl_gen->addRow("Num threads", le_threads);
  fl_gen->addRow("GUI update batch", le_gui_update_batch);
  // NOTE just leaving this setting always default: fl_gen->addRow("Prune half tree", cb_prune_half);
  fl_gen->addRow("Prune by cost", cb_prune_by_cost);
  fl_gen->addRow("No viewer update", cb_no_dtv);
  fl_gen->addRow("Pie chart show visited only", cb_no_pie);
  fl_gen->addRow("Verbose", cb_verbose);
  fl_gen->addRow("Sanity check", cb_sanity_check);

  QVBoxLayout *vl_main = new QVBoxLayout();
  vl_main->addLayout(fl_gen);
  vl_main->addWidget(pb_run_partitioner);

  setLayout(vl_main);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}
