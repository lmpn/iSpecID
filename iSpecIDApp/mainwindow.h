#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUndoStack>
#include <QTimer>
#include <QMovie>
#include <QFutureWatcher>
#include "ispecid.h"
#include "qrecord.h"
#include "dbconnection.h"
#include "graphscene.h"

using namespace ispecid::datatypes;


using namespace ispecid::datatypes;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTableView;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString app_dir, QWidget *parent = nullptr);
    ~MainWindow();
    void loadRecords();
    void setupRecordsTable();
    void setupOriginalResultsTable();
    void setupCurrentResultsTable();
    void setupGraphScene();
    void gradingTableAdjust(QTableView *tableView);
    void deleteRecordRows();
    void enableMenuDataActions(bool enable);
    void showGradingErrors(std::vector<std::string> &errors);
    void updateApp();
    void loading(bool load, QString text="");
    QString createCompleter();
    void exportDataToTSVHelper(QString dir_path, bool full = true);
    void saveProjectHelper();
    void exportResultsHelper(QString path);
    void exportDistancesHelper(QString path);

signals:
    void updateGraph();
    void updateColorGraph();
    void annotateFinish();
    void updateRecords();
    void updateResults();
    void showComponent(QString);
    void updateCurrentResults();
    void saveGraph(QString);
    void postQuery(QString);
    void loadFinished(int,int,int,int);
    void saveFinished();
    void errorOccured();
    void stopLoading();
    void error(QString, QString);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void scaleView(qreal scaleFactor);

private slots:
    void onError(QString error_type, QString error);
    void onStopLoading();
    void onSaveAsProject();
    void onSaveFinished();
    void onLoadFinished(int,int,int,int);
    void onAnnotateFinished();
    void onAnnotate();
    void onErrorOccured();
    void exportDataToTSV(bool);
    void loadDistanceMatrix();
    void onActionPerformed();
    void onComboBoxChanged();
    void zoomIn();
    void zoomOut();
    void onLoadProject();
    void onDeleteProject();
    void onExportResults();
    void onExportDistanceMatrix();
    void onSaveProject();
    void onNewProject();
    void saveGraph();
    void showGradingOptions();
    void autoCorrection();
    void showFilter();
    void onSaveConfig(double, int, int);
    void onRecordTableClick(const QModelIndex&  index);


private:
    QString app_dir;
    QString project;
    GradingParameters params;
    DistanceMatrix distances;
    std::vector<QRecord>* data;
    GraphScene* graph;
    Ui::MainWindow* ui;
    QUndoStack* undoStack;
    QMovie* movie;
    QTimer* timer;
    std::vector<std::string> errors;
    ispecid::IEngine* engine;
    bool save_distances = true;
};

#include <QUndoCommand>


//! [0]
class ActionCommand : public QUndoCommand
{
public:

    ActionCommand(std::vector<QRecord> *data, std::vector<QRecord> old,
                  QUndoCommand *parent = nullptr): QUndoCommand(parent){
        this->old = old;
        this->current = *data;
        this->data = data;
    }


    void undo() override{
        *data = old;
    };
    void redo() override{
        *data = current;
    };

private:
    std::vector<QRecord> *data;
    std::vector<QRecord> old;
    std::vector<QRecord> current;
};





#endif // MAINWINDOW_H
