#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QKeyEvent>
#include "graphviewer.h"
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
    void keyPressEvent(QKeyEvent *event) override;
    void removeRows();
    ~MainWindow();
signals:
    void updateGraph();
    void updateColorGraph();
    void updateRecords();
    void updateResults();
    void showComponent(QString);

private slots:

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
    GraphViewer *graph;
};
#endif // MAINWINDOW_H
