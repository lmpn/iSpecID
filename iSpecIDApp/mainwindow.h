#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QKeyEvent>
#include <annotator.h>
#include "graphviewer.h"
#include "resultsmodel.h"
#include "recordmodel.h"
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

private slots:

    void on_load_file_triggered();

    void on_graph_combo_box_activated(const QString &arg1);

private:
    Ui::MainWindow *ui;
    GraphViewer *graph;
    Annotator *an;
};
#endif // MAINWINDOW_H
