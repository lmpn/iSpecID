#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCompleter>
#include <string>
#include <functional>
#include <qdebug.h>
#include <QGraphicsView>
#include <QMessageBox>
#include "filterdialog.h"


void MainWindow::setupGraphScene(RecordModel *record_model, ResultsModel *results_model)
{
    graph = new GraphScene(this, engine);
    ui->graph_viewer->setScene(graph);
    ui->graph_viewer->setCacheMode(QGraphicsView::CacheBackground);
    ui->graph_viewer->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::BoundingRectViewportUpdate);
    ui->graph_viewer->setRenderHint(QPainter::Antialiasing);
    ui->graph_viewer->setTransformationAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
    connect(this, SIGNAL(saveGraph(QString)),
            graph,SLOT(onSaveGraph(QString)));
    connect(this, SIGNAL(updateGraph()),
            graph,SLOT(onGraphChange()));
    connect(this, SIGNAL(updateColorGraph()),
            graph,SLOT(onGraphColorChange()));
    connect(this, SIGNAL(showComponent(QString)),
            graph,SLOT(setComponentVisible(QString)));
    connect(graph, SIGNAL(updateComboBox()),
            this,SLOT(onComboBoxChange()));
    connect(graph, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChange()));
    connect(graph, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChange()));
    connect(record_model, SIGNAL(updateGraph()),
            graph, SLOT(onGraphChange()));
}

void MainWindow::enableMenuDataActions(bool enable)
{
    auto actions = ui->menuData->actions();
    for(auto ac : actions){
        ac->setEnabled(enable);
    }
}

void MainWindow::onComboBoxChange(){
    auto first = createCompleter();
    emit showComponent(first);
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
    ResultsModel *results_model = new ResultsModel(ui->results_table,engine);
    ui->results_table->setModel(results_model);
    ui->results_frame->hide();
    connect(this, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChange()));

    //Setup current results table
    ResultsModel *current_results_model = new ResultsModel(ui->current_results_table,engine);
    ui->current_results_table->setModel(current_results_model);
    ui->current_results_frame->hide();
    connect(this, SIGNAL(updateCurrentResults()),
            current_results_model,SLOT(onResultsChange()));

    //Setup graph
    setupGraphScene(record_model, current_results_model);

    //Disable actions of menubar
    enableMenuDataActions(false);
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
        engine->load(filePath.toStdString());
        auto r = engine->countFilterBadEntries();
        QMessageBox msgBox;
        msgBox.setText("Loading report.");
        msgBox.setInformativeText(QString("Number of removed entries due to empty species name: %1\n\nNumber of removed entries due to empty bin: %2\n\nNumber of removed entries due to empty institution: %3").arg(r["species"]).arg(r["bin"]).arg(r["institution"]));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        engine->group();
        auto first = createCompleter();
        enableMenuDataActions(true);
        emit updateRecords();
        emit updateGraph();
        emit showComponent(first);
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
    emit showComponent(arg1);
}



void MainWindow::on_annotateButton_clicked()
{
    engine->annotate();
    engine->gradeRecords();
    emit updateColorGraph();
    emit updateRecords();
    if(!ui->results_frame->isVisible()){
        ui->results_frame->show();
        emit updateResults();
    }
    ui->current_results_frame->show();
    emit updateCurrentResults();
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
    /*
    std::vector<std::function<bool(Record)>> preds;
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
    delete ff;*/
}



/*
    USER INPUT HANDLE
*/

void MainWindow::deleteRecordRows(){
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

void MainWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Output directory"), "");
    emit saveGraph(path);
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
