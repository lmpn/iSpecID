#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCompleter>
#include <string>
#include <functional>
#include <qdebug.h>
#include <QGraphicsView>
#include <QMessageBox>
#include <QScrollBar>
#include "filterdialog.h"


void MainWindow::setupGraphScene(RecordModel *record_model, ResultsModel *results_model)
{
    graph = new GraphScene(this, engine);
    ui->graph_viewer->setScene(graph);
    ui->graph_viewer->setCacheMode(QGraphicsView::CacheBackground);
    ui->graph_viewer->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::BoundingRectViewportUpdate);
    ui->graph_viewer->setRenderHint(QPainter::Antialiasing);
    ui->graph_viewer->setTransformationAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    connect(this, SIGNAL(save_graph(QString)),
            graph,SLOT(on_save_graph(QString)));
    connect(this, SIGNAL(update_graph()),
            graph,SLOT(on_graph_change()));
    connect(this, SIGNAL(update_color_graph()),
            graph,SLOT(on_graph_color_change()));
    connect(this, SIGNAL(show_component(QString)),
            graph,SLOT(set_component_visible(QString)));
    connect(graph, SIGNAL(update_combobox()),
            this,SLOT(on_combobox_changed()));
    connect(graph, SIGNAL(action_performed()),
            this,SLOT((on_action_performed)));
    connect(graph, SIGNAL(update_results()),
            results_model,SLOT(on_results_changed()));
    connect(graph, SIGNAL(update_records()),
            record_model,SLOT(on_records_changed()));
    connect(record_model, SIGNAL(update_graph()),
            graph, SLOT(on_graph_change()));
}

void MainWindow::enableMenuDataActions(bool enable)
{
    auto actions = ui->menu_data->actions();
    for(auto ac : actions){
        ac->setEnabled(enable);
    }
}

void MainWindow::on_combobox_changed(){
    auto first = createCompleter();
    emit show_component(first);
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



    //Setup record table
    RecordModel *record_model = new RecordModel(ui->record_table,engine);
    ui->record_table->setModel(record_model);
    ui->record_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    connect(this, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChange()));
    connect(record_model, SIGNAL(updateComboBox()),
            this, SLOT(onComboBoxChange()));
    //Setup results table
    ResultsModel *results_model = new ResultsModel(ui->initial_results_table,engine);
    ui->initial_results_table->setModel(results_model);
    gradingTableAdjust(ui->initial_results_table);
    ui->initial_results_frame->hide();
    connect(this, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChange()));

    //Setup current results table
    ResultsModel *current_results_model = new ResultsModel(ui->current_results_table,engine);
    ui->current_results_table->setModel(current_results_model);
    gradingTableAdjust(ui->current_results_table);
    ui->current_results_frame->hide();
    connect(this, SIGNAL(updateCurrentResults()),
            current_results_model,SLOT(onResultsChange()));

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


void MainWindow::on_load_file_triggered()
{
    //read file
    QString filePath = QFileDialog::getOpenFileName(this, tr("Input file"), "", tr("All Files (*)"));
    if (filePath.isEmpty())
        return;
    else {
        undoEntries = {};
        engine->load(filePath.toStdString());
        auto r = engine->countFilterBadEntries();
        QMessageBox msgBox;
        msgBox.setText("Loading report.");
        msgBox.setInformativeText(QString("Number of removed entries due to empty species name: %1\n\nNumber of removed entries due to empty bin: %2\n\nNumber of removed entries due to empty institution: %3").arg(r["species"]).arg(r["bin"]).arg(r["institution"]));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        updateApp();

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
    unique.sort();
    QCompleter * completer = new QCompleter(unique, this);
    ui->graph_combo_box->clear();
    ui->graph_combo_box->setCompleter(completer);
    ui->graph_combo_box->addItems(unique);
    ui->graph_combo_box->update();
    if(unique.size()>0)
        return unique.first();
    return QString();
}

void MainWindow::on_graph_combo_box_activated(const QString &arg1)
{
    //Change graph component
    emit show_component(arg1);
}



void MainWindow::on_annotateButton_clicked()
{
    if(engine->size() == 0) return;
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
    engine->annotate();
    engine->gradeRecords();
    emit update_color_graph();
    emit update_records();
    if(!ui->results_frame->isVisible()){
        ui->results_frame->show();
        emit update_results();
    }
    ui->current_results_frame->show();
    emit update_current_results();
}

void MainWindow::on_filter_triggered()
{
    QStringList headers;
    auto model = ui->record_table->model();
    auto count = model->columnCount();
    for(int i = 0; i < count; i++)
    {
        headers << model->headerData(i, Qt::Horizontal).toString();
    }
    auto ff = new FilterDialog(headers);
    ff->exec();
    if(ff->accepted()){
        undoEntries = engine->getEntriesCopy();
        undoFilteredEntries = engine->getFilteredEntriesCopy();
        auto pred = ff->getFilterFunc<Record>();
        engine->filter(pred);
        updateApp();

    }
}



/*
    USER INPUT HANDLE
*/

void MainWindow::deleteRecordRows(){
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
    emit update_current_results();
    emit update_graph();
    emit show_component(first);
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

/*
#if QT_CONFIG(wheelevent)
void MainWindow::wheelEvent(QWheelEvent *event)
{
    scaleView(pow(2., -event->angleDelta().y() / 240.0));
}
#endif
*/
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

void MainWindow::on_saveGraphButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save "), "",
                                                    tr("(*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {
        emit save_graph(fileName);
    }
}

void MainWindow::on_save_triggered()
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

void MainWindow::on_action_performed(){
    undoEntries = engine->getEntriesCopy();
    undoFilteredEntries = engine->getFilteredEntriesCopy();
}


void MainWindow::updateApp()
{
    engine->group();
    emit update_graph();
    emit update_records();
    emit update_current_results();
    auto first = createCompleter();
    emit show_component(first);
}

void MainWindow::on_undoButton_clicked()
{
    if(undoEntries.size() == 0) return;
    engine->setEntries(undoEntries);
    engine->setFilteredEntries(undoFilteredEntries);
    undoEntries = {};
    undoFilteredEntries = {};
    updateApp();
}

/*
void MainWindow::updateApp(){
    engine->group();
    emit update_graph();
    emit update_records();
    emit update_current_results();
    auto first = createCompleter();
    emit show_component(first);
}
*/
