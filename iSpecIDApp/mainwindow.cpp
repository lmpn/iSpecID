#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCompleter>
#include <string>
#include <functional>
#include <qdebug.h>
#include <QGraphicsView>
#include <QMessageBox>
#include <QScrollBar>
#include <qtconcurrentrun.h>
#include "filterdialog.h"
#include "gradingoptionsdialog.h"
#include <iostream>
#include <QtSql>
#include "csv.hpp"


void MainWindow::setupGraphScene(RecordModel *record_model, ResultsModel *results_model)
{
    graph = new GraphScene(this, engine);
    graph->setObjectName("graph_scene");
    ui->graph_viewer->setScene(graph);
    ui->graph_viewer->setCacheMode(QGraphicsView::CacheBackground);
    ui->graph_viewer->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::BoundingRectViewportUpdate);
    ui->graph_viewer->setRenderHint(QPainter::Antialiasing);
    ui->graph_viewer->setTransformationAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    connect(this, SIGNAL(updateGraph()),
            graph,SLOT(onGraphChanged()));
    connect(this, SIGNAL(updateColorGraph()),
            graph,SLOT(onGraphColorChanged()));
    connect(this, SIGNAL(showComponent(QString)),
            graph,SLOT(setComponentVisible(QString)));
    connect(graph, SIGNAL(updateCombobox()),
            this,SLOT(on_combo_box_changed()));
    connect(graph, SIGNAL(actionPerformed()),
            this,SLOT(onActionPerformed()));
    connect(graph, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChanged()));
    connect(graph, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChanged()));
    connect(record_model, SIGNAL(updateGraph()),
            graph, SLOT(onGraphChanged()));
}

void MainWindow::enableMenuDataActions(bool enable)
{
    auto actions = ui->menu_data->actions();
    for(auto ac : actions){
        ac->setEnabled(enable);
    }
}

void MainWindow::on_combo_box_changed(){
    auto first = createCompleter();
    emit showComponent(first);
}

void MainWindow::gradingTableAdjust(QTableView *tableView)
{
    tableView->resizeRowsToContents();
    int count=tableView->verticalHeader()->count();
    int horizontalHeaderHeight=tableView->horizontalHeader()->height();
    int rowTotalHeight=0;
    for (int i = 0; i < count; ++i) {
        rowTotalHeight+=tableView->verticalHeader()->sectionSize(i);
    }
    tableView->setMinimumHeight(horizontalHeaderHeight+rowTotalHeight+2);
    tableView->setMaximumHeight(horizontalHeaderHeight+rowTotalHeight+2);

    tableView->resizeColumnsToContents();
    count=tableView->horizontalHeader()->count();
    int columnTotalWidth=0;
    for (int i = 0; i < count; ++i) {
        tableView->setColumnWidth(i, tableView->horizontalHeader()->sectionSize(i)+2);
        columnTotalWidth+=tableView->horizontalHeader()->sectionSize(i);
    }
    tableView->setMinimumWidth(columnTotalWidth+2);
    tableView->setMaximumWidth(columnTotalWidth+2);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    engine = new IEngine();



    //conect buttons
    connect(ui->annotate_button, SIGNAL(clicked()),
            this, SLOT(onAnnotateData()));
    connect(ui->undo_button, SIGNAL(clicked()),
            this, SLOT(undo()));
    connect(ui->save_graph_button, SIGNAL(clicked()),
            this, SLOT(saveGraph()));
    connect(ui->zoom_out_button, SIGNAL(clicked()),
            this, SLOT(zoomOut()));
    connect(ui->zoom_in_button, SIGNAL(clicked()),
            this, SLOT(zoomIn()));
    connect(ui->load_file_action, SIGNAL(triggered()),
            this, SLOT(loadFile()));
    connect(ui->save_file_action, SIGNAL(triggered()),
            this, SLOT(saveFile()));
    //f
    connect(ui->filter_action, SIGNAL(triggered()),
            this, SLOT(showFilter()));
    connect(ui->gradind_options_action, SIGNAL(triggered()),
            this, SLOT(showGradingOptions()));
    //gopts




    //Setup record table
    RecordModel *record_model = new RecordModel(ui->record_table,engine);
    ui->record_table->setModel(record_model);
    ui->record_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    connect(this, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChanged()));
    connect(record_model, SIGNAL(updateCombobox()),
            this, SLOT(on_combo_box_changed()));
    connect(ui->record_table->horizontalHeader(), SIGNAL(sectionClicked( int )),
            record_model, SLOT(sortBySection(int)));

    //Setup results table
    ResultsModel *results_model = new ResultsModel(ui->initial_results_table,engine);
    ui->initial_results_table->setModel(results_model);
    gradingTableAdjust(ui->initial_results_table);
    ui->initial_results_frame->hide();
    connect(this, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChanged()));

    //Setup current results table
    ResultsModel *current_results_model = new ResultsModel(ui->current_results_table,engine);
    ui->current_results_table->setModel(current_results_model);
    gradingTableAdjust(ui->current_results_table);
    ui->current_results_frame->hide();
    connect(this, SIGNAL(updateCurrentResults()),
            current_results_model,SLOT(onResultsChanged()));

    ui->results_frame->hide();

    //Setup graph
    setupGraphScene(record_model, current_results_model);

    //Disable actions of menubar
    enableMenuDataActions(false);
    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete engine;
}


void MainWindow::loadFile()
{
    //read file
    QString filePath = QFileDialog::getOpenFileName(this, tr("Input file"), "", tr("All Files (*)"));
    if (filePath.isEmpty())
        return;
    else {
        undoEntries = {};
        undoFilteredEntries = {};
        engine->load(filePath.toStdString());
        auto r = engine->countFilterBadEntries();
        QMessageBox msgBox;
        msgBox.setText("Loading report.");
        msgBox.setInformativeText(QString("Number of removed entries due to empty species name: %1\n\nNumber of removed entries due to empty bin: %2\n\nNumber of removed entries due to empty institution: %3").arg(r["species"]).arg(r["bin"]).arg(r["institution"]));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        updateApp();
        enableMenuDataActions(true);
        ui->record_table->resizeColumnsToContents();
        msgBox.exec();
    }
}




QString MainWindow::createCompleter(){
    auto group_data = engine->getGroupedEntries();
    QSet<QString> set;
    for(auto& pair : group_data){
        set << QString::fromStdString(pair.first);
        for(auto& bin : pair.second.bins){
            set << QString::fromStdString(bin.first);
        }
    }
    auto unique = set.values();
    auto current = ui->graph_combo_box->currentText();
    unique.sort();
    QCompleter * completer = new QCompleter(unique, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->graph_combo_box->clear();
    ui->graph_combo_box->setCompleter(completer);
    ui->graph_combo_box->addItems(unique);
    ui->graph_combo_box->update();
    if(unique.contains(current)){
        ui->graph_combo_box->setCurrentText(current);
        return current;
    }
    if(unique.size()>0){
        auto first = unique.first();
        ui->graph_combo_box->setCurrentText(first);
        return first;
    }
    return QString();
}

void MainWindow::on_graph_combo_box_activated(const QString &arg1)
{
    //Change graph component
    emit showComponent(arg1);
}


void MainWindow::showGradingErrors(std::vector<std::string> &errors){
    QMessageBox msgBox;
    msgBox.setText("Grading error report.");
    QString error_str;
    for(auto& error : errors){
        error_str += QString::fromStdString(error);
        error_str += QString::fromStdString("\n");
    }
    msgBox.setDetailedText(error_str);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::annotateFinished(){
    emit updateColorGraph();
    emit updateRecords();
    if(!ui->results_frame->isVisible()){
        ui->results_frame->show();
        ui->initial_results_frame->show();
        ui->current_results_frame->show();
        emit updateResults();
    }
    emit updateCurrentResults();
    ui->annotate_button->setDisabled(false);
    ui->statusbar->showMessage("");
    if(errors.size()>0)
        showGradingErrors(this->errors);
    this->setCursor(Qt::ArrowCursor);
    ui->centralwidget->setEnabled(true);
}


void MainWindow::updateApp()
{
    engine->group();
    emit updateGraph();
    emit updateRecords();
    emit updateCurrentResults();
    auto first = createCompleter();
    emit showComponent(first);
}

/*
    USER INPUT HANDLE
*/

void MainWindow::deleteRecordRows()
{
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
    auto selection = ui->record_table->selectionModel()->selectedRows();
    auto model = (RecordModel *)ui->record_table->model();
    QList<int> rows;
    model->remove = true;
    for (QModelIndex index : selection) {
        rows.append(index.row());
    }
    std::sort(rows.begin(), rows.end(),  std::greater<int>());
    for (auto row : rows) {
        model->removeRow(row);
    }
    model->remove = false;
    engine->group();
    auto first = createCompleter();
    emit updateCurrentResults();
    emit updateGraph();
    emit showComponent(first);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace:
        deleteRecordRows();
        break;
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::scaleView(qreal scaleFactor)
{
    qreal factor = this->ui->graph_viewer->transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    this->ui->graph_viewer->scale(scaleFactor, scaleFactor);
}

void MainWindow::zoomIn()
{
    scaleView(qreal(1.2));
}

void MainWindow::zoomOut()
{
    scaleView(1 / qreal(1.2));
}

void MainWindow::saveGraph()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save "), "",
                                                    tr("(*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {
        const QPixmap pixmap = ui->graph_viewer->grab();
        pixmap.save(fileName);
    }
}

void MainWindow::saveFile()
{
    //QString path = QFileDialog::getExistingDirectory(this, tr("Output directory"), "");
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save "), "",
                                                    tr("Tab separated file (*.tsv);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {
        engine->save(fileName.toStdString());
    }
}

void MainWindow::onActionPerformed()
{
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
}

void MainWindow::undo()
{
    if(undoEntries.size() == 0) return;
    engine->setEntries(undoEntries);
    engine->setFilteredEntries(undoFilteredEntries);
    undoEntries = {};
    undoFilteredEntries = {};
    updateApp();
}

void MainWindow::onSaveConfig(double max_dist, int min_labs, int min_seqs)
{
    engine->setDist(max_dist);
    engine->setLabs(min_labs);
    engine->setDeposit(min_seqs);
}

void MainWindow::showGradingOptions()
{
    GradingOptionsDialog * gopts = new GradingOptionsDialog();
    connect(gopts, SIGNAL(saveConfig(double, int, int)),
            this, SLOT(onSaveConfig(double, int, int)));
    gopts->exec();
    delete gopts;

}

void MainWindow::onAnnotateData()
{
    if(engine->size() == 0){
        return;
    }
    this->setCursor(Qt::WaitCursor);
    ui->annotate_button->setDisabled(true);
    ui->statusbar->showMessage("Grading...");
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
    ui->centralwidget->setDisabled(true);
    QtConcurrent::run([this]{
        QObject src;
        engine->annotate(this->errors);
        engine->gradeRecords();
        QObject::connect(&src, SIGNAL(destroyed(QObject*)),
                         this, SLOT(annotateFinished()));
    });
}

void MainWindow::showFilter()
{
    QStringList headers;
    auto model = ui->record_table->model();
    auto count = model->columnCount();
    for(int i = 0; i < count; i++)
    {
        headers << model->headerData(i, Qt::Horizontal, Qt::DecorationRole).toString();
    }
    QSet<QString> species, bins, inst, grade;
    grade << "A" << "B" << "C" << "D" <<"E";
    auto entries = engine->getEntries();
    for(auto& entry: entries){
        species << QString::fromStdString(entry["species_name"]);
        bins << QString::fromStdString(entry["bin_uri"]);
        inst << QString::fromStdString(entry["institution_storing"]);
    }
    QList<QStringList> completions = {
        species.values(),
        bins.values(),
        inst.values(),
        grade.values()};
    auto ff = new FilterDialog(headers,completions);
    ff->exec();
    if(ff->accepted()){
        auto currentEntries = engine->getEntriesCopy();
        auto currentFilteredEntries = engine->getFilteredEntriesCopy();
        auto pred = ff->getFilterFunc<Record>();
        engine->filter(pred);
        int removedCount = engine->getFilteredEntriesCopy().size() - currentFilteredEntries.size();
        QMessageBox msgBox;
        msgBox.setText("Filter report.");
        msgBox.setInformativeText(QString("Number of removed: %1").arg(removedCount));
        msgBox.setStandardButtons(QMessageBox::No|QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        int apply = msgBox.result();
        qDebug() << apply;
        if(apply == 1024){
            undoEntries = currentEntries;
            undoFilteredEntries = currentFilteredEntries;
            updateApp();
        }
        else{
            engine->setEntries(currentEntries);
            engine->setFilteredEntries(currentFilteredEntries);
        }

    }
}

