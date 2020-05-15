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
#include <QMovie>
#include <QTimer>
#include "csv.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    engine = new IEngine();

    watcher = new QFutureWatcher<void>();

    graph = nullptr;

    movie = new QMovie(":/icons/dna_cursor.gif");
    timer = new QTimer(this);
    timer->setInterval(200); //Time in milliseconds
    //timer->setSingleShot(false); //Setting this to true makes the timer run only once
    connect(timer, &QTimer::timeout, this, [=](){
        QPixmap pix = movie->currentPixmap();
        movie->jumpToNextFrame();
        QCursor cs = QCursor(pix);
        this->setCursor(cs);
    });
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
    connect(ui->save_as_file_action, SIGNAL(triggered()),
            this, SLOT(saveAsFile()));
    connect(ui->save_file_action, SIGNAL(triggered()),
            this, SLOT(saveFile()));
    connect(ui->filter_action, SIGNAL(triggered()),
            this, SLOT(showFilter()));
    connect(ui->gradind_options_action, SIGNAL(triggered()),
            this, SLOT(showGradingOptions()));
    ui->results_frame->hide();
    enableMenuDataActions(false);
    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete engine;
    delete watcher;
    delete movie;
    delete timer;
}


void MainWindow::enableMenuDataActions(bool enable)
{
    auto actions = ui->menu_data->actions();
    for(auto ac : actions){
        ac->setEnabled(enable);
    }
}

void MainWindow::onComboBoxChanged(){
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
        tableView->setColumnWidth(i, tableView->horizontalHeader()->sectionSize(i)+4);
        columnTotalWidth+=tableView->horizontalHeader()->sectionSize(i);
    }
    tableView->setMinimumWidth(columnTotalWidth);
    tableView->setMaximumWidth(columnTotalWidth);
}



void MainWindow::setupGraphScene()
{
    if(graph != nullptr){
        delete graph;
    }

    auto record_model = ui->record_table->model();
    auto current_results_model = ui->current_results_table->model();
    graph = new GraphScene(this, engine);
    ui->graph_viewer->setScene(graph);
    ui->graph_viewer->setCacheMode(QGraphicsView::CacheBackground);
    ui->graph_viewer->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::BoundingRectViewportUpdate);
    ui->graph_viewer->setRenderHint(QPainter::Antialiasing);
    ui->graph_viewer->setTransformationAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    connect(this, SIGNAL(updateGraph()),
            graph,SLOT(onGraphChanged()));
    connect(this, SIGNAL(updateColorGraph()),
            graph,SLOT(onGraphColorChanged()));
    connect(ui->graph_combo_box, SIGNAL(currentIndexChanged(const QString&)),
            graph,SLOT(setComponentVisible(QString)));
    connect(graph, SIGNAL(updateCombobox()),
            this,SLOT(onComboBoxChanged()));
    connect(graph, SIGNAL(actionPerformed()),
            this,SLOT(onActionPerformed()));
    connect(graph, SIGNAL(updateResults()),
            current_results_model,SLOT(onResultsChanged()));
    connect(graph, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChanged()));
    connect(record_model, SIGNAL(updateGraph()),
            graph, SLOT(onGraphChanged()));
}

void MainWindow::setupOriginalResultsTable(){
    auto model = ui->initial_results_table->model();
    if(model != nullptr){
        delete model;
    }
    ResultsModel *results_model = new ResultsModel(engine);
    ui->initial_results_table->setModel(results_model);
    gradingTableAdjust(ui->initial_results_table);
    ui->initial_results_frame->hide();
    connect(this, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChanged()));
}

void MainWindow::setupCurrentResultsTable(){
    ResultsModel *current_results_model = new ResultsModel(engine);
    auto model = ui->current_results_table->model();
    if(model != nullptr){
        delete model;
    }
    ui->current_results_table->setModel(current_results_model);
    gradingTableAdjust(ui->current_results_table);
    ui->current_results_frame->hide();
    connect(this, SIGNAL(updateCurrentResults()),
            current_results_model,SLOT(onResultsChanged()));
}

void MainWindow::setupRecordsTable(){
    RecordModel *record_model = new RecordModel(engine);
    auto model = ui->record_table->model();
    if(model != nullptr){
        delete model;
    }
    ui->record_table->setModel(record_model);
    ui->record_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    connect(this, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChanged()));
    connect(record_model, SIGNAL(updateCombobox()),
            this, SLOT(onComboBoxChanged()));
    connect(ui->record_table->horizontalHeader(), SIGNAL(sectionClicked( int )),
            record_model, SLOT(sortBySection(int)));
}


void MainWindow::loadFileFinish(){

    disconnect(watcher, SIGNAL(finished()), this, SLOT(loadFileFinish()));
    auto r = engine->countFilterBadEntries();
    updateApp();
    ui->record_table->resizeColumnsToContents();
    ui->results_frame->hide();
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setText("Loading report.");
    msgBox->setInformativeText(
                QString("Number of removed entries due to empty species name: %1\n\nNumber of removed entries due to empty bin: %2\n\nNumber of removed entries due to empty institution: %3")
                .arg(r["species"])
            .arg(r["bin"])
            .arg(r["institution"])
            );
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setDefaultButton(QMessageBox::Ok);
    timer->stop();
    this->setCursor(Qt::ArrowCursor);
    ui->centralwidget->setEnabled(true);
    enableMenuDataActions(true);
    ui->statusbar->showMessage("");
    ui->menubar->setEnabled(true);
    msgBox->exec();
    msgBox->deleteLater();
}

void MainWindow::loadFile()
{
    //read file
    QFileDialog* qfd = new QFileDialog(this, tr("Input file"), QDir::homePath(), tr("All Files (*)"));
    qfd->setFileMode(QFileDialog::ExistingFile);
    int result = qfd->exec();
    QString filePath;
    if(result == QDialog::Accepted){
        auto files = qfd->selectedFiles();
        if(files.count() == 1){
            filePath = files[0];
        }
    }

//    QString filePath = QFileDialog::getOpenFileName(this, tr("Input file"), "", tr("All Files (*)"));
    if (filePath.isEmpty())
        return;
    else {
        current_save_path = "";
        engine->clear();
        //Setup record table
        setupRecordsTable();

        //Setup results table
        setupOriginalResultsTable();

        //Setup current results table
        setupCurrentResultsTable();

        //Setup graph
        setupGraphScene();

        undoEntries = {};
        undoFilteredEntries = {};
//        this->setCursor(Qt::WaitCursor);
        ui->statusbar->showMessage("Loading file...");
        ui->centralwidget->setDisabled(true);
        ui->menubar->setDisabled(true);

        connect(watcher, SIGNAL(finished()), this, SLOT(loadFileFinish()));
        timer->start();
        auto end = QtConcurrent::run([filePath, this]{
            engine->load(filePath.toStdString());
        });
        watcher->setFuture(end);
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

/*
void MainWindow::showGraphComponent(const QString &arg1)
{
    //Change graph component
    emit showComponent(arg1);
}
*/


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

    User Section

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
    emit updateCurrentResults();
    emit updateGraph();
    auto first = createCompleter();
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

/*
    End User Section
*/




/*

  Annotate Section

*/
void MainWindow::onAnnotateData()
{
    if(engine->size() == 0){
        return;
    }
//    this->setCursor(Qt::WaitCursor);
    timer->start();
    ui->statusbar->showMessage("Grading...");
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
    ui->centralwidget->setDisabled(true);
    ui->menubar->setDisabled(true);
    connect(watcher, SIGNAL(finished()), this, SLOT(annotateFinished()));
    auto end = QtConcurrent::run([this]{
        QObject src;
        QObject::connect(&src, SIGNAL(destroyed(QObject*)),
                         this, SLOT(annotateFinished()));
        engine->annotate(this->errors);
        engine->gradeRecords();
    });
    watcher->setFuture(end);
}

void MainWindow::showGradingErrors(std::vector<std::string> &errors){
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setText("Grading error report.");
    QString error_str;
    for(auto& error : errors){
        error_str += QString::fromStdString(error);
        error_str += QString::fromStdString("\n");
    }
    msgBox->setDetailedText(error_str);
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
    msgBox->deleteLater();
}

void MainWindow::annotateFinished(){
    disconnect(watcher, SIGNAL(finished()), this, SLOT(annotateFinished()));
    emit updateColorGraph();
    emit updateRecords();
    if(!ui->results_frame->isVisible()){
        ui->results_frame->show();
        ui->initial_results_frame->show();
        ui->current_results_frame->show();
        emit updateResults();
    }
    emit updateCurrentResults();
    timer->stop();
    ui->statusbar->showMessage("");
    if(errors.size()>0)
        showGradingErrors(this->errors);
    this->setCursor(Qt::ArrowCursor);
    ui->centralwidget->setEnabled(true);
    ui->menubar->setEnabled(true);
    this->errors = {};
}

/*

  End Annotate Section

*/

/*

  Save Section

*/

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

void MainWindow::saveAsFile()
{
    current_save_path = QFileDialog::getSaveFileName(
                this,
                tr("Save "),
                "",
                tr("Tab separated file (*.tsv);;All Files (*)"));
    if (current_save_path.isEmpty()){
        return;
    }
    engine->save(current_save_path.toStdString());
}

void MainWindow::saveFile()
{
    if(current_save_path.isEmpty()){
        current_save_path = QFileDialog::getSaveFileName(
                    this,
                    tr("Save "),
                    "",
                    tr("Tab separated file (*.tsv);;All Files (*)"));
        if (current_save_path.isEmpty()){
            return;
        }
    }
    engine->save(current_save_path.toStdString());
}
/*

  End Save Section

*/



/*

  Undo Section

*/

void MainWindow::undo()
{
    if(undoEntries.size() == 0) return;
    engine->setEntries(undoEntries);
    engine->setFilteredEntries(undoFilteredEntries);
    undoEntries = {};
    undoFilteredEntries = {};
    updateApp();
}

void MainWindow::onActionPerformed()
{
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
}


/*

  End Undo Section

*/





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
        grade.values()
    };
    auto ff = new FilterDialog(headers,completions);
    ff->exec();
    if(ff->accepted()){
        auto currentEntries = engine->getEntriesCopy();
        auto currentFilteredEntries = engine->getFilteredEntriesCopy();
        auto pred = ff->getFilterFunc<Record>();
        engine->filter(pred);
        int removedCount = engine->getFilteredEntriesCopy().size() - currentFilteredEntries.size();
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setText("Filter report.");
        msgBox->setInformativeText(QString("Number of removed: %1").arg(removedCount));
        msgBox->setStandardButtons(QMessageBox::No|QMessageBox::Ok);
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        int apply = msgBox->result();
        if(apply == 1024){
            undoEntries = currentEntries;
            undoFilteredEntries = currentFilteredEntries;
            updateApp();
        }
        else{
            engine->setEntries(currentEntries);
            engine->setFilteredEntries(currentFilteredEntries);
        }
        msgBox->deleteLater();
    }
    ff->deleteLater();
}

void MainWindow::showGradingOptions()
{
    GradingOptionsDialog * gopts = new GradingOptionsDialog();
    connect(gopts, SIGNAL(saveConfig(double, int, int)),
            this, SLOT(onSaveConfig(double, int, int)));
    gopts->exec();
    delete gopts;

}

void MainWindow::onSaveConfig(double max_dist, int min_labs, int min_seqs)
{
    engine->setDist(max_dist);
    engine->setLabs(min_labs);
    engine->setDeposit(min_seqs);
}
