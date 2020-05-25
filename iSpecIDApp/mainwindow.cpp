#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QInputDialog>
#include <QKeyEvent>
#include <qdebug.h>
#include <QCompleter>
#include <qtconcurrentrun.h>
#include "projectselectiondialog.h"
#include "qrecord.h"
#include "recordmodel.h"
#include "resultsmodel.h"
#include "gradingoptionsdialog.h"
#include "filterdialog.h"
#include "utils.h"


MainWindow::MainWindow(QString app_dir, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->app_dir = app_dir;
    undoStack = new QUndoStack(this);
    graph = nullptr;
    movie = new QMovie(":/icons/dna_cursor.gif");
    timer = new QTimer(this);
    timer->setInterval(200); //Time in milliseconds
    connect(timer, &QTimer::timeout, this, [=](){
        QPixmap pix = movie->currentPixmap();
        movie->jumpToNextFrame();
        QCursor cs = QCursor(pix);
        this->setCursor(cs);
    });
    connect(ui->new_project_action, SIGNAL(triggered()),
            this, SLOT(onNewProject()));
    connect(ui->load_project_action, SIGNAL(triggered()),
            this, SLOT(onLoadProject()));
    connect(ui->save_project_action, SIGNAL(triggered()),
            this, SLOT(onSaveProject()));
    connect(ui->save_graph_button, SIGNAL(clicked()),
            this, SLOT(saveGraph()));
    connect(ui->zoom_out_button, SIGNAL(clicked()),
            this, SLOT(zoomOut()));
    connect(ui->zoom_in_button, SIGNAL(clicked()),
            this, SLOT(zoomIn()));
    connect(ui->filter_action, SIGNAL(triggered()),
            this, SLOT(showFilter()));
    connect(ui->gradind_options_action, SIGNAL(triggered()),
            this, SLOT(showGradingOptions()));
    connect(ui->annotate_button, SIGNAL(clicked()),
            this, SLOT(onAnnotate()));
    connect(ui->undo_button, &QPushButton::clicked,
            [this](){
        if(undoStack->count() >0) {
            undoStack->undo();
            updateApp();
        }});
    connect(this, SIGNAL(loadFinished(int,int,int)), this,SLOT(onLoadFinished(int,int,int)));
    connect(this, SIGNAL(saveFinished()), this,SLOT(onSaveFinished()));
    connect(this, SIGNAL(stopLoading()), this,SLOT(onStopLoading()));
    connect(this, SIGNAL(errorOccured()), this,SLOT(onErrorOccured()));
    connect(this, SIGNAL(annotateFinish()), this,SLOT(onAnnotateFinished()));
    connect(ui->export_to_TSV_action, SIGNAL(triggered()), this,SLOT(exportDataToTSV()));
    connect(ui->load_distance_matrix_action, SIGNAL(triggered()), this,SLOT(loadDistanceMatrix()));
    ui->results_frame->hide();
    enableMenuDataActions(false);
    data = new std::vector<QRecord>();
    showMaximized();
}

void MainWindow::onStopLoading(){
    loading(false, "");
}

void MainWindow::exportDataToTSV(){
    if(data->size() == 0){
        QMessageBox::critical( this, "Export error", "Error no data found", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    QString dir_path = QFileDialog::getExistingDirectory(this,"Save directory",QDir::homePath(),QFileDialog::DontUseNativeDialog);
    if(dir_path.isEmpty()) {
        return;
    }

    loading(true, "Exporting project...");
    QtConcurrent::run(std::function<void(QString)>(
                          [this](QString dir_path){
                          qDebug() << dir_path;
                          QString filename = dir_path + QDir::separator() + project + ".tsv";
                          qDebug() << filename;
                          DbConnection dbc(app_dir);
                          if(!dbc.createConnection()){
                              QMessageBox::critical( this, "Export error", "SQLite3 error", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
                              return;
                          }
                          QString valuesStat = "select * from %1";
                          valuesStat = valuesStat.arg(project);
                          QString headerStat = "SELECT group_concat(name) FROM PRAGMA_TABLE_INFO('%1')";
                          headerStat = headerStat.arg(project);
                          QFile file(filename);
                          if (!file.open(QIODevice::WriteOnly)) {
                              QString error = "File %1 couldn't be open.";
                              error = error.arg(filename);
                              QMessageBox::critical(this, "File open error", error, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                              return;
                          }
                          QTextStream stream(&file);
                          auto header = dbc.execQuery(headerStat)[0];
                          auto values = dbc.execQuery(valuesStat);
                          stream << header.join("\t") + "\n";
                          for(auto& row : values){
                              stream << row.join("\t")+ "\n";
                          }
                          file.close();
                          emit stopLoading();
                      }), dir_path);

}

void MainWindow::loadDistanceMatrix(){
    QString filename = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::homePath(), tr("SV (*.csv *.tsv)"), 0, QFileDialog::DontUseNativeDialog );
    if(filename.isEmpty()){
        return;
    }
    loading(true, "Loading file...");
    QtConcurrent::run(
                std::function<void(QString)>([this](QString filename){
                    QFile file(filename);
                    QChar delim;
                    if (!file.open(QIODevice::ReadOnly)) {
                        QString error = "File %1 couldn't be open.";
                        error = error.arg(filename);
                        QMessageBox::critical(this, "File open error", error, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                        emit stopLoading();
                        return;
                    }
                    if(filename.contains("csv")){
                        delim = ';';
                    }
                    else if(filename.contains("tsv")){
                        delim = '\t';
                    }else{
                        QMessageBox::critical( this, "Load file error", "Incorrect format file", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
                        file.close();
                        emit stopLoading();
                        return;
                    }
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        QString line = in.readLine();
                        auto elems = line.split(delim);
                        if(elems.size() == 3){
                            bool ok;
                            double distance = elems[2].toDouble(&ok);
                            if (ok){
                                Neighbour neighbour({elems[0].toStdString(), elems[1].toStdString(), distance});
                                distances.insert({elems[0].toStdString(), neighbour});
                            }

                        }
                    }
                    file.close();
                    emit stopLoading();
                }
                ),filename);
}

void MainWindow::onErrorOccured(){
    QCoreApplication::exit(1);
}

void MainWindow::onAnnotate(){
    onActionPerformed();
    loading(true, "Grading...");
    std::vector<Record> records;
    std::for_each(data->begin(),data->end(),[&records](QRecord item){records.push_back(item.record);});
    auto dataset = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    QtConcurrent::run(
                std::function<void(Dataset&)>(
                    [this](Dataset& dataset){
                    this->errors = engine.annotate(dataset,this->distances,this->params);
                    for(auto& item : *data){
                        std::string key = item.record.getSpeciesName();
                        auto grade_it = dataset.find(key);
                        if(grade_it != dataset.end()){
                            item.record.setGrade(grade_it->second.getGrade());
                        }
                    }
                    emit annotateFinish();
                }
                ),dataset);

}
void MainWindow::onAnnotateFinished(){
    if(!errors.empty()){
        showGradingErrors(errors);
    }
    if(!ui->results_frame->isVisible()){
        ui->results_frame->show();
        ui->initial_results_frame->show();
        ui->current_results_frame->show();
        emit updateResults();
    }
    for(auto& d : distances){
        auto& sd = d.second;
        qDebug() << QString::fromStdString(sd.clusterA) << QString::fromStdString(sd.clusterB) << sd.distance;
    }
    emit updateColorGraph();
    emit updateCurrentResults();
    loading(false);
}

void MainWindow::onNewProject()
{
    bool ok;
    QString new_project = QInputDialog::getText( this, tr("New project"), tr("Project name:"), QLineEdit::Normal, "", &ok );
    if(!ok){
        return;
    }
    else if(ok && new_project.isEmpty()){
        QMessageBox::critical( this, "New project error", "Error trying to create project", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        emit errorOccured();
    }
    else if(ok && !new_project.isEmpty()){
        QString stat = "select * from projects where name = \"%1\"";
        stat = stat.arg(new_project);
        DbConnection dbc(app_dir);
        if(dbc.createConnection()){
            auto list = dbc.execQuery(stat);
            if(dbc.success() && list.size() > 0){
                QMessageBox::critical( this, "New project error", "Project already exists", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
                return;
            }
            if(!dbc.success()){
                QMessageBox::critical( this, "SQLite error", "Creating project error", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
                emit errorOccured();
                return;
            }
        }
        QString filename = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::homePath(), tr("SV (*.csv *.tsv)"), 0, QFileDialog::DontUseNativeDialog );
        if(filename.isEmpty()){
            return;
        }


        project = new_project;
        loading(true, "Loading file...");
        QtConcurrent::run(
                    std::function<void(QString)>([this](QString filename){
                        DbConnection dbc(app_dir);
                        if(dbc.createConnection()){
                            QString newProjectStat = "insert into projects (name) values (\"%1\")";
                            newProjectStat = newProjectStat.arg(project);
                            dbc.execQuery(newProjectStat);
                            if(!dbc.success()){
                                QMessageBox::critical(this, "SQLite error", "Error project", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                                emit errorOccured();
                                return;
                            }
                        }else{
                            QMessageBox::critical(this, "SQLite error", "Error project", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                            emit errorOccured();
                            return;
                        }
                        QFile file(filename);
                        if (!file.open(QIODevice::ReadOnly)) {
                            QString error = "File %1 couldn't be open.";
                            error = error.arg(filename);
                            QMessageBox::critical(this, "File open error", error, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                            emit errorOccured();
                            return;
                        }
                        QString stat;
                        QChar delim;
                        if(filename.contains("csv")){
                            delim = ';';
                        }
                        else if(filename.contains("tsv")){
                            delim = '\t';
                        }else{
                            QMessageBox::critical( this, "Load file error", "Incorrect format file", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
                            file.close();
                            emit loadFinished(0,0,0);
                            return;
                        }
                        int current_size = 0;
                        QTextStream in(&file);
                        QStringList header;
                        bool no_grade = false;
                        bool no_mod = false;

                        if(!in.atEnd()){
                            QString line = in.readLine();
                            header = line.split(delim);
                            auto species = header.contains("species_name");
                            auto bin = header.contains("bin_uri");
                            auto institution = header.contains("institution_storing");
                            if(!species || !bin || !institution){
                                QMessageBox::critical(this, "File parse error", "File header misses species_name, bin_uri or institution_storing", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                                file.close();
                                emit loadFinished(0,0,0);
                                return;
                            }
                            if(!header.contains("grade")) {
                                header << "grade";
                                no_grade = true;
                            }
                            if(!header.contains("modification")){
                                header << "modification";
                                no_mod = true;
                            }
                        }
                        QString header_join = header.join(",");
                        stat = "create table %1 ";
                        stat += "(" + header_join +")";
                        stat = stat.arg(project);
                        dbc.execQuery(stat);
                        if(!dbc.success()){
                            QMessageBox::critical(this, "SQLite error", "Error creating table", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                            emit errorOccured();
                            return;
                        }
                        QString base_stat = "insert into %1 (" + header_join + ") values ";
                        base_stat = base_stat.arg(project);
                        stat = base_stat;
                        while (!in.atEnd()) {
                            QString line = in.readLine();
                            QString line2 = line.replace("\"","");
                            line = line.replace("\t", "\",\"");
                            if(no_grade){
                                line += "\",\"U";
                            }
                            if(no_mod){
                                line += "\",\"";

                            }
                            line = "\"" + line + "\"";
                            stat += "(" + line +")";
                            current_size++;
                            if(current_size == 500){
                                dbc.execQuery(stat);
                                if(!dbc.success()){
                                    QMessageBox::critical(this, "SQLite error", "Error inserting in table", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                                    emit errorOccured();
                                    return;
                                }
                                stat = base_stat;
                                current_size = 0;
                            }else{
                                stat +=",";
                            }
                        }
                        stat.chop(1);
                        dbc.execQuery(stat);
                        if(!dbc.success()){
                            QMessageBox::critical(this, "SQLite error", "Error inserting in table", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
                            emit errorOccured();
                            return;
                        }
                        file.close();
                        loadRecords();
                    }
                    ),filename);

    }

}

void MainWindow::onLoadProject()
{
    DbConnection dbc(app_dir);
    QStringList projects;
    if(dbc.createConnection()){
        auto result = dbc.execQuery("select name from projects");
        for(auto& record : result){
            projects << record.first();
        }
    }else{
        QMessageBox::critical( this, "Load project error", "There was an error loading the project names", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    ProjectSelectionDialog *psd = new ProjectSelectionDialog(projects, this);
    int exit = psd->exec();
    if(exit == 1){
        project = psd->getProject();
        if(!project.isEmpty()){
            loading(true, "Loading project");
            QtConcurrent::run(std::function<void(void)>([this](){
                loadRecords();
            }
            ));
        }
    }
    psd->deleteLater();
}

void MainWindow::loading(bool load, QString text){
    if(load){
        enableMenuDataActions(false);
        ui->statusbar->showMessage(text);
        ui->centralwidget->setDisabled(true);
        ui->menubar->setDisabled(true);
        timer->start();
    }else{
        enableMenuDataActions(true);
        ui->centralwidget->setEnabled(true);
        ui->statusbar->showMessage("");
        ui->menubar->setEnabled(true);
        timer->stop();
        this->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::loadRecords(){
    DbConnection dbc(app_dir);
    int bad_species = 0;
    int bad_cluster = 0;
    int bad_source = 0;
    if(dbc.createConnection()){
        data->clear();
        undoStack->clear();
        ui->results_frame->hide();
        QSqlQuery query;
        QString loadStat = "select species_name , bin_uri, institution_storing, grade, count(*), group_concat(processid) from %1 group by species_name, bin_uri, institution_storing;";
        loadStat = loadStat.arg(project);
        auto result = dbc.execQuery(loadStat);
        if(!dbc.success()){
            QMessageBox::critical(this, "SQLite error", "Error inserting in table", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
            emit errorOccured();
            return;
        }
        for(auto& record : result){
            QString species_name = record[0];
            QString cluster = record[1];
            QString source =  record[2];
            QString grade = record[3];
            int count =record[4].toInt();
            QStringList ids = record[5].split(",");
            if(species_name.isEmpty()) bad_species+=count;
            else if(cluster.isEmpty()) bad_cluster+=count;
            else if(source.isEmpty()) bad_source+=count;
            else if(!grade.isEmpty()){
                Record rec(species_name.toStdString(),cluster.toStdString(),source.toStdString(),grade.toStdString(),count);
                QRecord qrec({ids, rec});
                data->push_back(qrec);
            }
        }
    }
    emit loadFinished(bad_species, bad_cluster, bad_source);
}

void MainWindow::onLoadFinished(int bad_species, int bad_cluster , int bad_source )
{
    setupRecordsTable();
    setupOriginalResultsTable();
    setupCurrentResultsTable();
    setupGraphScene();
    enableMenuDataActions(true);
    updateApp();
    loading(false);
    QString warning = "%1 records were filtered:\n%2 with empty species name\n%3 with empty cluster name\n%4 with empty source";
    warning = warning.arg(bad_source+bad_species+bad_cluster).arg(bad_species).arg(bad_cluster).arg(bad_source);
    QMessageBox::warning( this, "Loading report", warning, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);

}

void MainWindow::onSaveFinished(){
    loading(false);
}

void MainWindow::onSaveProject(){
    loading(true, "Saving project...");
    QtConcurrent::run(std::function<void(void)>([this](){
        DbConnection dbc(app_dir);
        if(dbc.createConnection()){
            QStringList updated_ids;
            for(auto& qrec : *data){
                updated_ids << qrec.ids;
                auto ids = qrec.ids.join("\",\"");
                QString updateStat="update %1 set species_name= \"%2\", bin_uri = \"%3\", institution_storing = \"%4\", grade = \"%5\" where processid in (\"%6\")";
                updateStat = updateStat
                        .arg(project)
                        .arg(QString::fromStdString(qrec.record.getSpeciesName()))
                        .arg(QString::fromStdString(qrec.record.getCluster()))
                        .arg(QString::fromStdString(qrec.record.getSource()))
                        .arg(QString::fromStdString(qrec.record.getGrade()))
                        .arg(ids);
                dbc.execQuery(updateStat);
                if (!dbc.success()) {
                    QMessageBox::critical( this, "Save project error", "There was an error saving the project", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
                }
            }
            QString updateStat="update %1 set grade = \"\" where processid not in (\"%2\")";
            updateStat = updateStat
                    .arg(project)
                    .arg(updated_ids.join("\",\""));
            dbc.execQuery(updateStat);
            if (!dbc.success()) {
                QMessageBox::critical( this, "Save project error", "There was an error saving the project", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
            }
            emit saveFinished();
        }}));
}

void MainWindow::setupRecordsTable(){
    RecordModel *record_model = new RecordModel(data);
    auto model = ui->record_table->model();
    if(model != nullptr){
        delete model;
    }
    ui->record_table->setModel(record_model);
    ui->record_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    connect(this, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChanged()));
    connect(ui->record_table->horizontalHeader(), SIGNAL(sectionClicked( int )),
            record_model, SLOT(sortBySection(int)));
    connect(record_model, SIGNAL(updateCombobox()),
            this, SLOT(onComboBoxChanged()));
}

void MainWindow::onComboBoxChanged(){
    auto first = createCompleter();
    //emit showComponent(first);
}

QString MainWindow::createCompleter(){
    QSet<QString> set;
    for(auto& rec : *data){
        set << QString::fromStdString(rec.record.getSpeciesName());
        set << QString::fromStdString(rec.record.getCluster());
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

void MainWindow::deleteRecordRows()
{
    auto selection_model = ui->record_table->selectionModel();
    if(selection_model == nullptr){
        return;
    }
    auto selection = selection_model->selectedRows();
    QList<int> rows;
    for (QModelIndex index : selection) {
        rows.append(index.row());
    }
    std::sort(rows.begin(), rows.end(),  std::less<int>());
    int removed = 0;
    if(rows.size() > 0){
        onActionPerformed();
        for (auto row : rows) {
            data->erase(data->begin()+row-removed);
            removed++;
        }
        updateApp();
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

void MainWindow::setupOriginalResultsTable()
{
    auto model = ui->initial_results_table->model();
    if(model != nullptr){
        delete model;
    }
    ResultsModel *results_model = new ResultsModel(data);
    ui->initial_results_table->setModel(results_model);
    gradingTableAdjust(ui->initial_results_table);
    ui->initial_results_frame->hide();
    connect(this, SIGNAL(updateResults()),
            results_model,SLOT(onResultsChanged()));
}

void MainWindow::setupCurrentResultsTable()
{
    auto model = ui->current_results_table->model();
    if(model != nullptr){
        delete model;
    }
    ResultsModel *current_results_model = new ResultsModel(data);
    ui->current_results_table->setModel(current_results_model);
    gradingTableAdjust(ui->current_results_table);
    ui->current_results_frame->hide();
    connect(this, SIGNAL(updateCurrentResults()),
            current_results_model,SLOT(onResultsChanged()));
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

void MainWindow::enableMenuDataActions(bool enable)
{
    auto actions = ui->menu_data->actions();
    for(auto ac : actions){
        ac->setEnabled(enable);
    }
}

void MainWindow::setupGraphScene()
{
    if(graph != nullptr){
        delete graph;
    }

    auto record_model = ui->record_table->model();
    auto current_results_model = ui->current_results_table->model();
    graph = new GraphScene(this, data);
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

void MainWindow::showFilter()
{
    QStringList headers;
    auto model = ui->record_table->model();
    auto count = model->columnCount();
    for(int i = 0; i < count-1; i++)
    {
        headers << model->headerData(i, Qt::Horizontal, Qt::DecorationRole).toString();
    }
    QSet<QString> species, bins, inst, grade;
    grade << "A" << "B" << "C" << "D" <<"E";
    for(auto& qrec: *data){
        species << QString::fromStdString(qrec.record.getSpeciesName());
        bins << QString::fromStdString(qrec.record.getCluster());
        inst << QString::fromStdString(qrec.record.getSource());
    }
    QList<QStringList> completions = {
        species.values(),
        bins.values(),
        grade.values(),
        inst.values()
    };
    auto ff = new FilterDialog(headers,completions);
    ff->exec();
    if(ff->accepted()){
        auto pred = ff->getFilterFunc();
        auto temp = std::vector<QRecord>(data->begin(), data->end());
        data->erase(std::remove_if(data->begin(), data->end(), pred), data->end());
        int removedCount = temp.size()-data->size();
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setText("Filter report.");
        msgBox->setInformativeText(QString("Number of removed records: %1").arg(removedCount));
        msgBox->setStandardButtons(QMessageBox::No|QMessageBox::Ok);
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        int apply = msgBox->result();
        if(apply == 1024){
            onActionPerformed();
            updateApp();
        }
        else{
            std::swap(*data, temp);
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
    gopts->deleteLater();
}

void MainWindow::onSaveConfig(double max_dist, int min_labs, int min_seqs)
{
    params.max_distance = max_dist;
    params.min_sources = min_labs;
    params.min_size = min_seqs;
}

void MainWindow::showGradingErrors(std::vector<std::string> &errors)
{
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

void MainWindow::onActionPerformed(){
    ActionCommand * action = new ActionCommand(data, *data);
    undoStack->push(action);
}

void MainWindow::updateApp()
{
    emit updateGraph();
    emit updateRecords();
    emit updateCurrentResults();
    auto first = createCompleter();
    emit showComponent(first);
}

MainWindow::~MainWindow()
{
    delete movie;
    delete timer;
    delete graph;
    delete data;
    delete ui;
}
