#include "iSpecIDApp/mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCompleter>
#include <qdebug.h>
#include <string>
#include "iSpecIDApp/filterform.h"
#include <functional>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    an = new Annotator();
    ResultsModel *res_m = new ResultsModel(ui->results_table,an);
    RecordModel *rec_m = new RecordModel(ui->record_table,an);
    ui->results_table->setModel(res_m);
    ui->record_table->setModel(rec_m);
    graph = new GraphViewer(ui->graph_viewer, an);


    connect(this, SIGNAL(updateRecords()),
            rec_m,SLOT(onRecordsChange()));

    connect(this, SIGNAL(updateResults()),
            res_m,SLOT(onResultsChange()));

    connect(this, SIGNAL(updateColorGraph()),
            graph,SLOT(onGraphColorChange()));
    connect(this, SIGNAL(updateGraph()),
            graph,SLOT(onGraphChange()));
    connect(this, SIGNAL(showComponent(QString)),
            graph,SLOT(setComponentVisible(QString)));


    connect(graph, SIGNAL(updateRecords()),
            rec_m,SLOT(onRecordsChange()));

    connect(graph, SIGNAL(updateResults()),
            res_m,SLOT(onResultsChange()));



    setupResultsTable();
    ui->record_table->resizeColumnsToContents();
    graph->show();
    ui->annotateButton->setEnabled(false);
    ui->results_frame->hide();
    auto actions = ui->menuData->actions();
    for(auto ac : actions){
        ac->setEnabled(false);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete an;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace:
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}


void MainWindow::on_load_file_triggered()
{
    //read file
    QString qfile_path = QFileDialog::getOpenFileName(this, tr("Input file"), "", tr("All Files (*)"));
    auto file_path = qfile_path.toStdString();
    if(file_path.empty()) return;
    an->cleanLoad(file_path);
    auto first = createCompleter();
    updateApp(first);
    ui->record_table->resizeColumnsToContents();
    ui->results_frame->show();
    ui->annotateButton->setEnabled(true);
    auto actions = ui->menuData->actions();
    for(auto ac : actions){
        ac->setEnabled(true);
    }
}



void MainWindow::updateApp(QString name){
    emit updateRecords();
    emit updateResults();
    emit updateGraph();
    if(!name.isEmpty()){
        emit showComponent(name);
    }
}




QString MainWindow::createCompleter(){
    auto group_data = an->getGroupRecords();
    QSet<QString> set;
    for(auto& pair : group_data){
        set << QString::fromStdString(pair.first);
        for(auto& bin_name : pair.second.bins){
            set << QString::fromStdString(bin_name);
        }

    }
    auto unique = set.values();
    unique.sort();
    QCompleter * completer = new QCompleter(unique, this);
    ui->graph_combo_box->setCompleter(completer);
    ui->graph_combo_box->clear();
    ui->graph_combo_box->addItems(unique);
    if(unique.size()>0)
        return unique.first();
    return QString();
}

void MainWindow::on_graph_combo_box_activated(const QString &arg1)
{
    graph->setComponentVisible(arg1);
}


void MainWindow::setupResultsTable(){
    ui->results_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->results_table->verticalHeader()->hide();
    int w = 0, h = 0;
    w += ui->results_table->contentsMargins().left() + ui->results_table->contentsMargins().right();
    h += ui->results_table->contentsMargins().top() + ui->results_table->contentsMargins().bottom();
    w += ui->results_table->verticalHeader()->width();
    h += ui->results_table->horizontalHeader()->height();

    for (int i=0; i<ui->results_table->model()->columnCount(); ++i)
        w += ui->results_table->columnWidth(i);
    for (int i=0; i<ui->results_table->model()->rowCount(); ++i)
        h += ui->results_table->rowHeight(i);

    ui->results_table->setMaximumWidth(w-16);
    ui->results_table->setMaximumHeight(h);
    ui->results_frame->setMaximumWidth(w + ui->record_frame->contentsMargins().left()+ui->record_frame->contentsMargins().right()*10);
    ui->results_frame->setMaximumHeight(h+ui->record_frame->rect().height()+ ui->record_frame->contentsMargins().bottom()+ui->record_frame->contentsMargins().top());
}







void MainWindow::on_annotateButton_clicked()
{
    an->annotationAlgo();
    an->gradeRecords();
    emit updateColorGraph();
    emit updateRecords();
    emit updateResults();
    ui->annotateButton->setEnabled(false);

}

void MainWindow::on_filter_triggered()
{
    std::vector<std::function<bool(Record)>> preds;
    auto ff = new FilterForm();
    ff->exec();
    if(ff->isAccepted()){
        auto species = ff->getSpecies().toStdString();
        auto bin = ff->getBin().toStdString();
        auto inst = ff->getInstitution().toStdString();
        auto grade = ff->getGrade().toStdString();
        if(!species.empty()){
            preds.push_back([species](Record item){
                return item["species_name"] == species;
            });
        }
        if(!bin.empty()){
            preds.push_back([bin](Record item){
                return item["bin_uri"] == bin;
            });
        }
        if(!inst.empty()){
            preds.push_back([inst](Record item){
                return item["institution_storing"] == inst;
            });
        }
        if(!grade.empty()){
            preds.push_back([grade](Record item){
                return item["grade"] == grade;
            });
        }
        int size = an->size();
        an->filter(preds, ff->getMatch());
        if(size != an->size()){
            an->clearGroup();
            an->group();
            an->calculateGradeResults();
            auto first = createCompleter();
            updateApp(first);
            ui->annotateButton->setEnabled(true);
        }
    }
    delete ff;
}
