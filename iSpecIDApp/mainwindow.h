#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QKeyEvent>
#include "graphscene.h"
#include "resultsmodel.h"
#include "recordmodel.h"
#include <annotator.h>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void removeRows();
    ~MainWindow();
signals:
    void updateGraph();
    void updateColorGraph();
    void updateRecords();
    void updateResults();
    void showComponent(QString);
protected:
    void keyPressEvent(QKeyEvent *event) override;
    #if QT_CONFIG(wheelevent)
        void wheelEvent(QWheelEvent *event) override;
    #endif
    void scaleView(qreal scaleFactor);

private slots:
    void zoomIn();
    void zoomOut();
    void on_load_file_triggered();
    void on_graph_combo_box_activated(const QString &arg1);
    void on_annotateButton_clicked();
    void on_filter_triggered();

private:
    QString createCompleter();
    void setupRecordTable();
    void setupResultsTable();
    void updateApp(QString name);
    Ui::MainWindow *ui;
    Annotator *an;
    GraphScene *graph;
};
#endif // MAINWINDOW_H
