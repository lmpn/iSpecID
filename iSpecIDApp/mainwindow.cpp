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
#include <QStringList>
#include <qtconcurrentrun.h>
#include "projectselectiondialog.h"
#include "qrecord.h"
#include "recordmodel.h"
#include "resultsmodel.h"
#include "gradingoptionsdialog.h"
#include "filterdialog.h"
#include "utils.h"
#include "fileio.h"


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
    connect(ui->record_table, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(onRecordTableClick(const QModelIndex& )));
    connect(ui->record_table, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(onRecordTableClick(const QModelIndex& )));
    connect(ui->new_project_action, SIGNAL(triggered()),
            this, SLOT(onNewProject()));
    connect(ui->export_distance_matrix_action, SIGNAL(triggered()),
            this, SLOT(onExportDistanceMatrix()));
    connect(ui->export_results_action, SIGNAL(triggered()),
            this, SLOT(onExportResults()));
    connect(ui->load_project_action, SIGNAL(triggered()),
            this, SLOT(onLoadProject()));
    connect(ui->delete_project_action, SIGNAL(triggered()),
            this, SLOT(onDeleteProject()));
    connect(ui->save_as_project_action, SIGNAL(triggered()),
            this, SLOT(onSaveAsProject()));
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
    connect(ui->auto_correction_action, SIGNAL(triggered()),
            this, SLOT(autoCorrection()));
    connect(ui->annotate_button, SIGNAL(clicked()),
            this, SLOT(onAnnotate()));
    connect(ui->undo_button, &QPushButton::clicked,
            [this](){
        if(undoStack->count() > 0) {
            undoStack->undo();
            updateApp();
        }});
    connect(this, SIGNAL(loadFinished(int,int,int,int)), this,SLOT(onLoadFinished(int,int,int,int)));
    connect(this, SIGNAL(saveFinished()), this,SLOT(onSaveFinished()));
    connect(this, SIGNAL(stopLoading()), this,SLOT(onStopLoading()));
    connect(this, SIGNAL(errorOccured()), this,SLOT(onErrorOccured()));
    connect(this, SIGNAL(annotateFinish()), this,SLOT(onAnnotateFinished()));
    connect(this, SIGNAL(error(QString,QString)), this,SLOT(onError(QString, QString)));
    connect(ui->export_to_TSV_action, &QAction::triggered, [&](){
        exportDataToTSV(false);
    });
    connect(ui->export_full_project_to_TSV_action, &QAction::triggered, [&](){
        exportDataToTSV(true);
    });
    connect(ui->load_distance_matrix_action, SIGNAL(triggered()), this,SLOT(loadDistanceMatrix()));
    connect(ui->close_project_action, &QAction::triggered, [this](){
        save_distances = true;
        project = "";
        data->clear();
        deleted->clear();
        distances.clear();
        undoStack->clear();
        ui->results_frame->hide();
        enableMenuDataActions(false);
        updateApp();
    });
    ui->results_frame->hide();
    enableMenuDataActions(false);
    data = new std::vector<QRecord>();
    deleted = new std::vector<QRecord>();
    engine = new ispecid::IEngine(4);
    showMaximized();

}


void MainWindow::autoCorrectionHelper(){
    std::vector<QRecord> records;
    for(auto &record: *data){
        if(record.ids.size() > 1){
            records.push_back(record);
        }
    }
    data->swap(records);
}

void MainWindow::autoCorrection(){
    onActionPerformed();
    loading("Auto correcting...");
    QtConcurrent::run(std::function<void(void)>(
                          [this](){
        autoCorrectionHelper();
        emit stopLoading();
                      }));
}
void MainWindow::onRecordTableClick(const QModelIndex& index){
    int col = index.column();
    if (col < 2){
        QString name = ui->record_table->model()->data(index).toString();
        ui->graph_combo_box->setCurrentText(name);
        emit showComponent(name);
    }
}

void MainWindow::onStopLoading(){
    loading(false, "");
    updateApp();
}

void MainWindow::onError(QString error_type, QString error){
    QMessageBox::critical( this, error_type, error, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
    onStopLoading();
}

void MainWindow::exportDataToTSVHelper(QString filename, bool full){
    DbConnection dbc(app_dir);
    if(!dbc.createConnection()){
        emit error("Export error", "SQLite3 error");
        return;
    }
    QString valuesStat;
    filename = filename.replace(".tsv","");
    QString params_filename = filename + "_params.txt";
    if(full){
        valuesStat = "select * from \"%1\"";
        filename = filename + ".tsv";
    }
    else{
        valuesStat = "select * from \"%1\" where modification not like \"%DELETED%\" and species_name != \"\" and institution_storing != \"\" and bin_uri != \"\"";
        filename = filename + "_pruned"+".tsv";
    }
    valuesStat = valuesStat.arg(project);
    QString headerStat = "SELECT group_concat(name) FROM PRAGMA_TABLE_INFO(\"%1\")";
    headerStat = headerStat.arg(project);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QString error_msg = "File %1 couldn't be open.";
        error_msg = error_msg.arg(filename);
        emit error("File open error", error_msg);
        return;
    }

    QFile file_params(params_filename);
    if (!file_params.open(QIODevice::WriteOnly)) {
        QString error_msg = "File %1 couldn't be open.";
        error_msg = error_msg.arg(params_filename);
        emit error("File open error", error_msg);
        return;
    }

    QString max_dist = "max_distance " + QString::number(params.max_distance);
    QString min_sources = "min_sources " + QString::number(params.min_sources);
    QString min_records = "min_records " + QString::number(params.min_size);
    auto params_str = max_dist +"\n" +min_sources +"\n" + min_records;
    file_params.write(params_str.toUtf8());
    file_params.close();

    QTextStream stream(&file);
    auto header = dbc.execQuery(headerStat)[0][0];
    int index = header.split(',').indexOf("grade");
    auto values = dbc.execQuery(valuesStat);
    stream << header.replace(",","\t")+"\n";
    QString grade_ast = "";
    if(params.min_sources == 1)
        grade_ast = "*";
    for(auto& row : values){
        row[index] += grade_ast;
        stream << row.join("\t")+ "\n";
    }
    file.close();
}

void MainWindow::exportDataToTSV(bool full){
    if(data->size() == 0){
        QMessageBox::critical( this, "Export error", "Error no data found", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    QString file_path = QFileDialog::getSaveFileName(this,
                                                     tr("Save Project"), "",
                                                     tr("Tab Separated File (*.tsv);;All Files (*)"),
                                                     0, QFileDialog::DontUseNativeDialog);
    if(file_path.isEmpty()) {
        return;
    }

    loading(true, "Exporting project...");
    QtConcurrent::run(std::function<void(QString, bool)>(
                          [this](QString filename, bool full){
                          saveProjectHelper();
                          exportDataToTSVHelper(filename, full);
                          emit stopLoading();
                      }), file_path, full);

}

void MainWindow::loadDistanceMatrix(){
    QString filename = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::homePath(), tr("SV (*.csv *.tsv)"), 0, QFileDialog::DontUseNativeDialog );
    if(filename.isEmpty()){
        return;
    }
    loading(true, "Loading file...");
    save_distances = false;
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
                                Neighbour neighbour_a({elems[0].toStdString(), elems[1].toStdString(), distance});
                                Neighbour neighbour_b({elems[1].toStdString(), elems[0].toStdString(), distance});
                                auto it_a = distances.find(elems[0].toStdString());
                                if(it_a != distances.end()){
                                    if(it_a->second.distance > distance){
                                        distances[elems[0].toStdString()] = neighbour_a;
                                    }
                                }else{
                                    distances[elems[0].toStdString()] = neighbour_a;
                                }
                                auto it_b = distances.find(elems[1].toStdString());
                                if(it_b != distances.end()){
                                    if(it_b->second.distance > distance){
                                        distances[elems[1].toStdString()] = neighbour_b;
                                    }
                                }else{
                                    distances[elems[1].toStdString()] = neighbour_b;
                                }
                            }

                        }
                    }
                    for(auto& pair: distances){
                        PRINT(pair.second.clusterA <<" "<< pair.second.clusterB <<" "<< pair.second.distance);

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
    if(data->size() == 0) return;
    onActionPerformed();
    loading(true, "Grading...");
    std::vector<Record> records;
    std::for_each(data->begin(),data->end(),[&records](QRecord item){records.push_back(item.record);});
    auto dataset = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    QtConcurrent::run(
                std::function<void(Dataset&)>(
                    [this](Dataset& dataset){
                    this->errors = engine->annotate(dataset,this->distances,this->params);
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
    emit updateColorGraph();
    emit updateCurrentResults();
    loading(false);
}

void MainWindow::onSaveAsProject(){
    bool ok;
    QString new_project = QInputDialog::getText( this, tr("Save project as"), tr("Project name:"), QLineEdit::Normal, "", &ok );
    if(!ok || project.isEmpty()){
        return;
    }
    else if(ok && new_project.isEmpty()){
        QMessageBox::critical( this, "New project error", "Error trying to create project", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
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
                return;
            }
        }

        QString newProjectStat = "insert into projects (name,max_dist,sources,records) values (\"%1\",%2,%3,%4)";
        newProjectStat = newProjectStat.arg(new_project).arg(params.max_distance).arg(params.min_sources).arg(params.min_size);;
        dbc.execQuery(newProjectStat);
        if(!dbc.success()){
            emit error("SQLite error", "Error project");
            return;
        }


        QString insertStat = "create table \"%1\" as select * from \"%2\";";
        insertStat = insertStat.arg(new_project);
        insertStat = insertStat.arg(project);
        dbc.execQuery(insertStat);
        if(!dbc.success()){
            emit error("SQLite error", "Error project");
            return;
        }
        project = new_project;
    }
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
        return;
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
                return;
            }
        }
        QString filename = QFileDialog::getOpenFileName( this, tr("Open File"), QDir::homePath(), tr("SV (*.txt *.tsv)"), 0, QFileDialog::DontUseNativeDialog );
        if(filename.isEmpty()){
            return;
        }


        project = new_project;
        loading(true, "Loading file...");
        QtConcurrent::run(
                    std::function<void(QString)>([this](QString filename){
                        DbConnection dbc(app_dir);

                        QFile file(filename);
                        if (!file.open(QIODevice::ReadOnly)) {
                            QString error_msg = "File %1 couldn't be open.";
                            error_msg = error_msg.arg(filename);
                            emit error("File open error", error_msg);
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
                            file.close();
                            emit error("Load file error", "Incorrect format file");
                            return;
                        }
                        int current_size = 0;
                        QTextStream in(&file);
                        QStringList header;
                        bool no_grade = false;
                        bool no_mod = false;
                        bool no_id = false;

                        if(!in.atEnd()){
                            QString line = in.readLine();
                            header = line.split(delim);
                            auto species = header.contains("species_name");
                            auto bin = header.contains("bin_uri");
                            auto institution = header.contains("institution_storing");
                            if(!species || !bin || !institution){
                                file.close();
                                emit error("Load file error", "Header doesn't have species_name, bin_uri or institution_storing");
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
                            if(!header.contains("ispecID")){

                                header.prepend("ispecID");
                                no_id = true;
                            }
                        }
                        if(dbc.createConnection()){
                            QString newProjectStat = "insert into projects (name, max_dist, sources, records) values (\"%1\", 2.0, 2, 3)";
                            newProjectStat = newProjectStat.arg(project);
                            dbc.execQuery(newProjectStat);
                            if(!dbc.success()){
                                emit error("SQLite error", "Error project");
                                return;
                            }
                        }else{
                            emit error("SQLite error", "Error project");
                            return;
                        }
                        QString header_join = header.join(",");
                        stat = "create table \"%1\" ";
                        stat += "(" + header_join +")";
                        stat = stat.arg(project);
                        dbc.execQuery(stat);
                        if(!dbc.success()){
                            emit error("SQLite error", "Error creating table");
                            return;
                        }
                        header[0] = header[0] + " primary key";
                        QString base_stat = "insert into \"%1\" (" + header_join + ") values ";
                        base_stat = base_stat.arg(project);
                        stat = base_stat;
                        int idx = 0;
                        while (!in.atEnd()) {
                            QString line = in.readLine();
                            auto line_split = line.replace("\"","").split("\t");
                            if(no_id)
                            {
                                line_split.prepend(QString::number(idx));}
                            line = line_split.join("\",\"");
                            if(no_grade){
                                line += "\",\"U";
                            }
                            if(no_mod){

                                line += "\",\"";

                            }
                            line = "\"" + line + "\"";
                            stat += "(" + line +")";
                            current_size++;
                            idx++;
                            if(current_size == 500){
                                dbc.execQuery(stat);
                                if(!dbc.success()){
                                    emit error("SQLite error", "Error inserting in table");
                                    return;
                                }
                                stat = base_stat;
                                current_size = 0;
                            }else{
                                stat +=",";
                            }
                        }
                        if(current_size != 0){
                            stat.chop(1);
                            dbc.execQuery(stat);
                            if(!dbc.success()){
                                emit error("SQLite error", "Error inserting in table");
                                return;
                            }

                        }
                        file.close();

                        QString updateSourceStat="update \"%1\" set modification= \"DELETED: empty source\" where institution_storing == \"\"";
                        updateSourceStat = updateSourceStat.arg(project);
                        dbc.execQuery(updateSourceStat);
                        if(!dbc.success()){
                            emit error("SQLite error", "Error inserting in table");
                            return;
                        }
                        QString updateClusterStat="update \"%1\" set modification= \"DELETED: empty cluster/bin_uri\" where bin_uri == \"\"";
                        updateClusterStat = updateClusterStat.arg(project);
                        dbc.execQuery(updateClusterStat);
                        if(!dbc.success()){
                            emit error("SQLite error", "Error inserting in table");
                            return;
                        }
                        QString updateSpeciesNameStat="update \"%1\" set modification= \"DELETED: empty name\" where species_name == \"\"";
                        updateSpeciesNameStat= updateSpeciesNameStat.arg(project);
                        dbc.execQuery(updateSpeciesNameStat);
                        if(!dbc.success()){
                            emit error("SQLite error", "Error inserting in table");
                            return;
                        }
                        loadRecords();
                    }
                    ),filename);

    }

}


void MainWindow::onLoadProject()
{
    DbConnection dbc(app_dir);
    QStringList projects;
    QList<GradingParameters> params_list;
    if(dbc.createConnection()){
        auto result = dbc.execQuery("select name, sources, records, max_dist from projects");
        for(auto& record : result){
            projects << record.first();
            GradingParameters gp = {record[1].toInt(), record[2].toInt(), record[3].toDouble()};
            params_list << gp;
        }
    }else{
        QMessageBox::critical( this, "Load project error", "There was an error loading the project names", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    LoadProjectSelectionDialog *psd = new LoadProjectSelectionDialog(app_dir, project, projects, this);
    int exit = psd->exec();
    if(exit == 1){
        save_distances = true;
        project = psd->selected_project;
        params = params_list[projects.indexOf(project)];
        loading(true, "Loading project");
        QtConcurrent::run(std::function<void(void)>([this](){
            loadRecords();
        }
        ));
    }
    psd->deleteLater();
}


void MainWindow::onDeleteProject()
{
    DbConnection dbc(app_dir);
    QStringList projects;
    QList<GradingParameters> params_list;
    if(dbc.createConnection()){
        auto result = dbc.execQuery("select name, sources, records, max_dist from projects");
        for(auto& record : result){
            projects << record.first();
            GradingParameters gp = {record[1].toInt(), record[2].toInt(), record[3].toDouble()};
            params_list << gp;
        }
    }else{
        QMessageBox::critical( this, "Load project error", "There was an error loading the project names", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    DeleteProjectSelectionDialog *psd = new DeleteProjectSelectionDialog(app_dir, project, projects, this);
    psd->exec();
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
    int deleted = 0;
    if(dbc.createConnection()){
        distances.clear();
        data->clear();
        undoStack->clear();
        ui->results_frame->hide();
        QSqlQuery query;
        QString loadStat = "select species_name , bin_uri, institution_storing, grade, count(*), group_concat(ispecID), modification from \"%1\" group by species_name, bin_uri, institution_storing;";
        loadStat = loadStat.arg(project);
        auto result = dbc.execQuery(loadStat);
        if(!dbc.success()){
            emit error("SQLite error", QString("Error loading in table %1").arg(project));
            return;
        }
        QSet<QString> clusters;
        for(auto& record : result){
            QString species_name = record[0];
            QString cluster = record[1];
            QString source =  record[2];
            QString grade = record[3];
            int count = record[4].toInt();
            QStringList ids = record[5].split(",");
            QString modification = record[6];
            if(species_name.isEmpty()) bad_species+=count;
            else if(cluster.isEmpty()) bad_cluster+=count;
            else if(source.isEmpty()) bad_source+=count;
            else if(modification == "DELETED by user;"){
                deleted+=count;
            }
            else{
                Record rec(species_name.toStdString(),cluster.toStdString(),source.toStdString(),grade.toStdString(),count);
                QRecord qrec({ids, modification, rec});
                clusters << cluster;
                data->push_back(qrec);
            }
        }
        auto cluster_ids = clusters.values();
        auto cluster_ids_str = cluster_ids.join("\",\"");
        QString clustersLoadStat = "select clusterA, clusterB, distance from neighbours where clusterA in (\"%1\") and strftime(\"%s\",\"now\") - time < 864000 ";
        clustersLoadStat = clustersLoadStat.arg(cluster_ids_str);
        result = dbc.execQuery(clustersLoadStat);
        if(!dbc.success()){
            emit error("SQLite error", "Error loading in neighbours");
            return;
        }
        for(auto& record : result){
            std::string clusterA = record[0].toStdString();
            std::string clusterB = record[1].toStdString();
            double distance =  record[2].toDouble();
            Neighbour neighbour = {clusterA,clusterB,distance};
            distances.insert({clusterA, neighbour});
        }
    }
    emit loadFinished(bad_species, bad_cluster, bad_source, deleted);
}

void MainWindow::onLoadFinished(int bad_species, int bad_cluster , int bad_source, int deleted )
{
    setupRecordsTable();
    setupOriginalResultsTable();
    setupCurrentResultsTable();
    setupGraphScene();
    enableMenuDataActions(true);
    updateApp();
    loading(false);
    QString warning = "%1 records were filtered:\n%2 with empty species name\n%3 with empty cluster name\n%4 with empty source\n%5 deleted by user";
    warning = warning.arg(bad_source+bad_species+bad_cluster).arg(bad_species).arg(bad_cluster).arg(bad_source).arg(deleted);
    QMessageBox::warning( this, "Loading report", warning, QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);

}

void MainWindow::onSaveFinished(){
    loading(false);
}

void MainWindow::saveProjectHelper(){
        DbConnection dbc(app_dir);
        if(dbc.createConnection()){
            QStringList updated_ids;
            for(auto& qrec : *data){
                auto ids = qrec.ids.join("\",\"");
                auto mod = qrec.modification;
                QString updateStat  = "update \"%1\" set species_name= \"%2\", bin_uri = \"%3\", institution_storing = \"%4\", grade = \"%5\", modification = \"%6\" where species_name= \"%7\" and bin_uri = \"%8\" and institution_storing = \"%9\"";
                updateStat = updateStat
                        .arg(project)
                        .arg(QString::fromStdString(qrec.record.getSpeciesName()))
                        .arg(QString::fromStdString(qrec.record.getCluster()))
                        .arg(QString::fromStdString(qrec.record.getSource()))
                        .arg(QString::fromStdString(qrec.record.getGrade()))
                        .arg(mod)
                        .arg(QString::fromStdString(qrec.record.getSpeciesName()))
                        .arg(QString::fromStdString(qrec.record.getCluster()))
                        .arg(QString::fromStdString(qrec.record.getSource()));
                dbc.execQuery(updateStat);
                if (!dbc.success()) {
                    emit error("Save project error", "There was an error saving the project");
                }
            }
            for(auto& qrec : *deleted){
                updated_ids << qrec.ids;
            }
            QString updateStat="update \"%1\" set modification = \"DELETED by user;\" where ispecID in (\"%2\")";
            updateStat = updateStat
                    .arg(project)
                    .arg(updated_ids.join("\",\""));
            dbc.execQuery(updateStat);
            if (!dbc.success()) {
                emit error("Save project error", "There was an error saving the project");
            }
            for(auto& neighbour : distances){
                QString updateStat="insert into neighbours(clusterA, clusterB, distance, time) values (\"%1\", \"%2\", \"%3\", strftime(\"%s\",\"now\")) "
                                   "ON CONFLICT(clusterA) DO UPDATE SET clusterB=\"%4\", distance=%5, time=strftime(\"%s\",\"now\")  where strftime(\"%s\",\"now\") - time > 864000";
                QString updateStatA = updateStat
                        .arg(QString::fromStdString(neighbour.second.clusterA))
                        .arg(QString::fromStdString(neighbour.second.clusterB))
                        .arg(neighbour.second.distance)
                        .arg(QString::fromStdString(neighbour.second.clusterB))
                        .arg(neighbour.second.distance);
                QString updateStatB = updateStat
                        .arg(QString::fromStdString(neighbour.second.clusterB))
                        .arg(QString::fromStdString(neighbour.second.clusterA))
                        .arg(neighbour.second.distance)
                        .arg(QString::fromStdString(neighbour.second.clusterA))
                        .arg(neighbour.second.distance);
                dbc.execQuery(updateStatA);
                if (!dbc.success()) {
                    emit error("Save project error", "There was an error saving the project");
                }
                dbc.execQuery(updateStatB);
                if (!dbc.success()) {
                    emit error("Save project error", "There was an error saving the project");
                }
            }
            QString paramsStat = "update projects set max_dist = %1, sources = %2, records = %3 where name = \"%4\"";
            paramsStat = paramsStat.arg(params.max_distance).arg(params.min_sources).arg(params.min_size);
            dbc.execQuery(paramsStat);
            if (!dbc.success()) {
                emit error("Save project error", "There was an error saving the project");
            }
    }
}

void MainWindow::onSaveProject(){
    loading(true, "Saving project...");
    if(project.isEmpty()) return;
    QtConcurrent::run(std::function<void(void)>([this](){
        saveProjectHelper();
        emit saveFinished();
    }));
}

void MainWindow::setupRecordsTable(){
    RecordModel *record_model = new RecordModel(data);
    auto model = ui->record_table->model();
    if(model != nullptr){
        disconnect(ui->record_table->horizontalHeader(), &QHeaderView::sectionClicked, model, nullptr);
        delete model;
    }
    ui->record_table->setModel(record_model);
    ui->record_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    connect(this, SIGNAL(updateRecords()),
            record_model,SLOT(onRecordsChanged()));
    connect(ui->record_table->horizontalHeader(), &QHeaderView::sectionPressed,
            [this](int column){
        (( RecordModel *)ui->record_table->model())->sortBySection(column);
        //        ui->record_table->resizeColumnsToContents();
    });
    connect(record_model, SIGNAL(updateCombobox()),
            this, SLOT(onComboBoxChanged()));
}

void MainWindow::onComboBoxChanged(){
    auto first = createCompleter();
    emit showComponent(first);
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
    ui->graph_combo_box->setInsertPolicy(QComboBox::NoInsert);
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
            deleted->push_back(data->operator[](row-removed));
            data->erase(data->begin()+row-removed);
            removed++;
        }
        updateApp();
    }
}

void MainWindow::scaleView(qreal scaleFactor)
{
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
                                                    tr("(*.png);;All Files (*)"),
                                                    0,
                                                    QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return;
    else {
        auto area = graph->sceneRect();
        auto image = new QImage(area.width()+100, area.height()+100, QImage::Format_ARGB32_Premultiplied);
        auto painter = new QPainter(image);
        painter->fillRect(image->rect(),Qt::white);
        painter->translate(10,10);
        graph->render(painter, image->rect(), area);
        painter->end();
        image->save(fileName+".png","PNG",100);
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
    connect(this, SIGNAL(showComponent(const QString&)),
            graph,SLOT(setComponentVisible(QString)));
    connect(ui->graph_combo_box, &QComboBox::currentTextChanged,  [this](){
        auto text = ui->graph_combo_box->currentText();
        auto count = ui->graph_combo_box->count();
        for (int index = 0; index < count;  index++){
            auto data = ui->graph_combo_box->itemText(index);
            if(data == text){
                graph->setComponentVisible(text);
                break;
            }
        }
    });
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
    QSet<QString> species, bins, inst;
    QStringList grade;
    grade << "A" << "B" << "C" << "D" <<"E";
    for(auto& qrec: *data){
        species << QString::fromStdString(qrec.record.getSpeciesName());
        bins << QString::fromStdString(qrec.record.getCluster());
        inst << QString::fromStdString(qrec.record.getSource());
    }
    QList<QStringList> completions = {
        species.values(),
        bins.values(),
        grade,
        inst.values()
    };
    auto ff = new FilterDialog(headers,completions);
    ff->exec();
    if(ff->accepted()){
        auto pred = ff->getFilterFunc();
        auto temp = std::vector<QRecord>(data->begin(), data->end());
        int removeCount = 0;
        std::for_each(temp.begin(), temp.end(),[&removeCount](QRecord rec){removeCount += rec.record.count();});
        temp.erase(std::remove_if(temp.begin(), temp.end(), pred), temp.end());
        std::for_each(temp.begin(), temp.end(),[&removeCount](QRecord rec){removeCount -= rec.record.count();});
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setText("Filter report.");
        msgBox->setInformativeText(QString("Number of removed records: %1").arg(removeCount));
        msgBox->setStandardButtons(QMessageBox::No|QMessageBox::Ok);
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        int apply = msgBox->result();
        if(apply == 1024){
            onActionPerformed();
            data->erase(std::remove_if(data->begin(), data->end(), pred), data->end());
            updateApp();
        }
        msgBox->deleteLater();
    }
    ff->deleteLater();
}

void MainWindow::showGradingOptions()
{
    qDebug() << params.max_distance << params.min_size << params.min_sources ;
    GradingOptionsDialog * gopts = new GradingOptionsDialog(this->params);
    connect(gopts, SIGNAL(saveConfig(double, int, int)),
            this, SLOT(onSaveConfig(double, int, int)));
    gopts->exec();
    gopts->deleteLater();
}

void MainWindow::onSaveConfig(double max_dist, int min_labs, int min_seqs)
{
    DbConnection dbc(app_dir);
    if(!dbc.createConnection()){
        emit error("Export error", "SQLite3 error");
        return;
    }
    params.max_distance = max_dist;
    params.min_sources = min_labs;
    params.min_size = min_seqs;
    QString paramsStat = "update projects set max_dist = %1, sources = %2, records = %3 where name = \"%4\"";
    paramsStat = paramsStat.arg(params.max_distance).arg(params.min_sources).arg(params.min_size).arg(project);
    dbc.execQuery(paramsStat);
    if (!dbc.success()) {
        emit error("Save project error", "There was an error saving the project");
    }
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
    ActionCommand * action = new ActionCommand(data, *data, deleted, *deleted);
    undoStack->push(action);
}

void MainWindow::updateApp()
{
    emit updateGraph();
    emit updateRecords();
    emit updateCurrentResults();
    ui->record_table->resizeColumnsToContents();
    auto first = createCompleter();
    emit showComponent(first);
}

void MainWindow::exportResultsHelper(QString path){
    std::vector<Record> records;
    std::for_each(data->begin(),data->end(),[&records](QRecord item){records.push_back(item.record);});
    auto dataset = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    std::map<QString, QStringList> bin_group;
    for(auto& rec: records){
        auto key = QString::fromStdString(rec.getCluster());
        auto value = QString::fromStdString(rec.getSpeciesName());
        if(!bin_group[key].contains(value)){
            bin_group[key] << value;
        }

    }


    auto results_filename = path+"/"+project+"_stats.tsv";
    QFile results_file(results_filename);
    if (!results_file.open(QIODevice::WriteOnly)) {
        QString error_msg = "File %1 couldn't be open.";
        error_msg = error_msg.arg(results_filename);
        emit error("File open error", error_msg);
        return;
    }

    QTextStream results_stream(&results_file);
    auto tab = "\t";
    auto model = ui->current_results_table->model();
    auto rows = model->rowCount();
    for(int i=0; i < rows; i++){
        QModelIndex grade_idx = model->index(i, 0, QModelIndex());
        QModelIndex count_idx = model->index(i, 1, QModelIndex());
        QModelIndex perc_idx = model->index(i, 2, QModelIndex());
        QString grade = model->data(grade_idx).toString();
        QString count = QString::number(model->data(count_idx).toInt());
        QString perc = QString::number(model->data(perc_idx).toDouble());
        results_stream << grade+tab+count+tab+perc+"\n";
    }
    results_stream.flush();
    results_file.close();
    /*
        Export stats
    */
    /*
        Export group by species
    */
    auto species_filename = path+"/"+project+"_species.tsv";
    QFile species_file(species_filename);
    if (!species_file.open(QIODevice::WriteOnly)) {
        QString error_msg = "File %1 couldn't be open.";
        error_msg = error_msg.arg(species_filename);
        emit error("File open error", error_msg);
        return;
    }

    QTextStream species_stream(&species_file);
    for(auto& species : dataset){
        auto sp = species.second;
        auto sp_clusters = sp.getClusters();
        auto row = QString::fromStdString(sp.getSpeciesName()) + tab
                + QString::number(sp.clustersCount()) + tab;
        for(auto cluster : sp_clusters)
            row += QString::fromStdString(cluster)+";";
        species_stream << row + "\n";
    }
    species_stream.flush();
    species_file.close();
    /*
        Export group by bins
    */
    auto bins_filename = path+"/"+project+"_bins.tsv";
    QFile bins_file(bins_filename);
    if (!bins_file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString error_msg = "File %1 couldn't be open.";
        error_msg = error_msg.arg(bins_filename);
        emit error("File open error", error_msg);
        return;
    }

    QTextStream bins_stream(&bins_file);
    for(auto& bin : bin_group){
        auto cluster = bin.first;
        auto species = bin.second;
        auto row = cluster + tab
                + QString::number(species.size()) + tab
                + species.join(";");
        bins_stream << row + "\n";
    }
    bins_stream.flush();
    bins_file.close();
}

void MainWindow::onExportResults(){
    if(data->size() == 0){
        QMessageBox::critical( this, "Export error", "Error no data found", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    QString path = QFileDialog::getExistingDirectory(
                this, tr("Save Project"), QDir::homePath(), QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    if(path.isEmpty()) {
        return;
    }

    loading(true, "Export results...");
    QtConcurrent::run(std::function<void(QString)>(
                          [this](QString path){
                          saveProjectHelper();
                          exportResultsHelper(path);
                          stopLoading();
                      }), path);
}

void MainWindow::exportDistancesHelper(QString path){

    QString filename = path+this->project + ".tsv";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QString error_msg = "File %1 couldn't be open.";
        error_msg = error_msg.arg(filename);
        emit error("File open error", error_msg);
        return;
    }

    QTextStream stream(&file);
    auto tab = "\t";
    for(auto& neighbour : distances){
        auto binA = QString::fromStdString(neighbour.second.clusterA);
        auto binB = QString::fromStdString(neighbour.second.clusterB);
        auto dist = QString::number(neighbour.second.distance);
        stream << binA+tab+binB+tab+dist+"\n";
    }
    file.close();
    emit stopLoading();
}

void MainWindow::onExportDistanceMatrix(){
    if(data->size() == 0){
        QMessageBox::critical( this, "Export error", "Error no data found", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok );
        return;
    }
    QString file_path = QFileDialog::getSaveFileName(this,
                                                     tr("Save Project"), "",
                                                     tr("Tab Separated File (*.tsv);;All Files (*)"),
                                                     0, QFileDialog::DontUseNativeDialog);
    if(file_path.isEmpty()) {
        return;
    }

      loading(true, "Export distances...");
    QtConcurrent::run(std::function<void(QString)>(
                          [this](QString filename){
                          saveProjectHelper();
                          exportDistancesHelper(filename);
                          stopLoading();
                      }), file_path);
}

MainWindow::~MainWindow()
{
    delete movie;
    delete timer;
    delete graph;
    delete data;
    delete ui;
    delete engine;
}
