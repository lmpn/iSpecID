#include "iSpecIDApp/mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCompleter>
#include <QDebug>
#include <string>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qan = new QAnnotator(this);
    setupResultsTable();
    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
    if(graph != nullptr){
        delete graph;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace:
        removeRows();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}


void MainWindow::removeRows(){

    QModelIndexList selection = ui->record_table->selectionModel()->selectedRows();
    auto record_m = ui->record_table->model();
    for(auto& idx : selection){
        record_m->removeRow(idx.row(),QModelIndex());
    }
}

void MainWindow::on_load_file_triggered()
{
    //read file
    QString qfile_path = QFileDialog::getOpenFileName(this, tr("Input file"), "", tr("All Files (*)"));
    auto file_path = qfile_path.toStdString();
    an->load(file_path);
    an->filter([](Record item) {return item["species_name"].empty();});
    an->group();
    auto results = an->getGradeResults();
    auto group_data = an->getGroupData();

    QSet<QString> set;
    for(auto& pair : group_data){
        set << QString::fromStdString(pair.first);
        for(auto& bin_name : pair.second.bins){
            set << QString::fromStdString(bin_name);
        }

    }
    //Results
    auto results_model = (ResultsModel*) ui->results_table->model();
    results_model->setResults(results);
    //Records
    auto unique = set.values();
    unique.sort();

    setupRecordTable(unique);

    if(graph != nullptr){
        delete graph;
    }
    graph = new GraphViewer( an, ui->graph_viewer);
    graph->show();
    graph->setComponentVisible(unique.first());
}

void MainWindow::on_graph_combo_box_activated(const QString &arg1)
{
    graph->setComponentVisible(arg1);
}



void MainWindow::setupRecordTable(QStringList unique){
    auto data = an->getData();
    QCompleter * completer = new QCompleter(unique, this);
    ui->graph_combo_box->setCompleter(completer);
    ui->graph_combo_box->addItems(unique);
    ui->record_table->setModel(new RecordModel(this, data));
    ui->record_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}





void MainWindow::setupResultsTable(){
    auto results_model = new ResultsModel();
    ui->results_table->setModel(results_model);
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
    an->annotation_algo();
}
