/*!
  \file partitioner_tests.cpp
  \brief Unit tests for the partitioner program, adapted from assignment 2.
  \author Samuel Ng
  \date 2021-02-15 created
  \copyright GNU LGPL v3
  */

#include <QtTest/QtTest>
#include<QtTest/QSignalSpy>
#include <QJsonObject>
#include "partitioner/partitioner.h"
#include "gui/settings.h"

class PartitionerTests : public QObject
{
  Q_OBJECT

  public:

    /*! \brief Read test problem properties from JSON.
     *
     * Read the test problem properties from the provided path and return a 
     * QVariantMap.
     */
    QVariantMap readTestProps(const QString &f_path)
    {
      QFile f(f_path);
      if (!f.open(QIODevice::ReadOnly)) {
        qFatal("Unable to read test problem properties in unit test.");
      }

      QTextStream f_text(&f);
      QString json_str;
      json_str = f_text.readAll();
      f.close();
      QByteArray json_bytes = json_str.toLocal8Bit();

      QJsonDocument json_doc = QJsonDocument::fromJson(json_bytes);

      if (json_doc.isNull()) {
        qFatal("Failed to create JSON doc in unit test.");
      }
      if (!json_doc.isObject()) {
        qFatal("JSON is not an object in unit test.");
      }

      QJsonObject json_obj = json_doc.object();

      if (json_obj.isEmpty()) {
        qFatal("JSON object is empty in unit test.");
      }

      return json_obj.toVariantMap();
    }


  // functions in these slots are automatically called after compilation
  private slots:

    //! Test that partitioning problems can be read successfully.
    void testProblemReadAndPartitioning()
    {
      using namespace sp;
      using namespace pt;

      // All test problems have an associated manually filled properties JSON 
      // file. The formats are always "name.txt" for the problem and 
      // "name_props.json" for the properties. Populate p_names by the list of
      // names. The actual files are located under src/qrc/test_problems.
      QStringList p_names;
      p_names << "atest2" << "atest3" << "atest4" << "baby";

      for (QString p_name : p_names) {
        QString base_name = ":/test_problems/" + p_name;
        QVariantMap expected_props = readTestProps(base_name + "_props.json");
        Graph graph(base_name + ".txt");
        // check that basic properties are read correctly
        QCOMPARE(graph.numBlocks(), expected_props["num_blocks"].value<int>());
        QCOMPARE(graph.numNets(), expected_props["num_nets"].value<int>());

        // check that partitioner returns the expected results
        PSettings pset;
        PartitionerBusyWrapper partitioner(graph, pset);
        PResults results = partitioner.runPartitioner();
        QCOMPARE(results.best_cut_size, expected_props["cut_size"]);
      }
    }
};

QTEST_MAIN(PartitionerTests)
#include "partitioner_tests.moc"  // generated at compile time
