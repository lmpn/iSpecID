#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QKeyEvent>
#include <iostream>

#include "graphscene.h"
#include "resultsmodel.h"
#include "recordmodel.h"
#include "annotator.h"
#include "iengine.h"


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
    void updateCurrentResults();
    void saveGraph(QString);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    /*
    #if QT_CONFIG(wheelevent)
        void wheelEvent(QWheelEvent *event) override;
    #endif*/
    void scaleView(qreal scaleFactor);

private slots:
    void zoomIn();
    void zoomOut();
    void on_load_file_triggered();
    void on_graph_combo_box_activated(const QString &arg1);
    void on_annotateButton_clicked();
    void on_filter_triggered();
    void onComboBoxChange();

    void on_pushButton_clicked();

    void on_save_triggered();

private:
    void deleteRecordRows();
    QString createCompleter();
    void updateApp(QString name);
    Ui::MainWindow *ui;
    IEngine *engine;

    GraphScene *graph;
    void setupGraphScene(RecordModel *rec_m, ResultsModel *res_m);
    void enableMenuDataActions(bool enable);
};
#endif // MAINWINDOW_H
