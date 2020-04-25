#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QKeyEvent>
#include <QtSql>
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
    void update_graph();
    void update_color_graph();
    void update_records();
    void update_results();
    void show_component(QString);
    void update_current_results();
    void save_graph(QString);

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
    void on_annotate_button_clicked();
    void on_filter_triggered();
    void on_combo_box_changed();
    void on_action_performed();
    void on_save_graph_button_clicked();
    void on_save_triggered();
    void on_undo_button_clicked();
    void on_save_config(double max_dist, int min_labs, int min_seqs);
    void on_gradind_options_action_triggered();
    void on_grading_finished();

private:
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
