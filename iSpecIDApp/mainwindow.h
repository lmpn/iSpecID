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

class QTableView;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
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
    void loadFile();
    void on_graph_combo_box_activated(const QString &arg1);
    void on_combo_box_changed();
    void onAnnotateData();
    void showFilter();
    void onActionPerformed();
    void saveGraph();
    void saveFile(QString path = "");
    void undo();
    void onSaveConfig(double max_dist, int min_labs, int min_seqs);
    void showGradingOptions();
    void annotateFinished();

private:
    QString save_path;
    Ui::MainWindow *ui;
    IEngine *engine;
    std::vector<Record> undoEntries;
    std::vector<Record> undoFilteredEntries;
    GraphScene *graph;
    std::vector<std::string> errors;
    void setupGraphScene(RecordModel *rec_m, ResultsModel *res_m);
    void enableMenuDataActions(bool enable);
    void updateApp();
    void gradingTableAdjust(QTableView *tableView);
    void deleteRecordRows();
    QString createCompleter();
    void removeRows();
    void showGradingErrors(std::vector<std::string> &errors);
};
#endif // MAINWINDOW_H
