#include "corrector.h"
#include "ui_corrector.h"
#include "iSpecIDCore/include/reader.hpp"
#include "iSpecIDCore/include/record.hpp"
#include "iSpecIDCore/include/annotator.hpp"

corrector::corrector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::corrector)
{
    ui->setupUi(this);
    setupGrades();
    an = new Annotator();
    //GraphWidget *widget = new GraphWidget( ui->graphicsView);
}

corrector::~corrector()
{
    delete an;
    delete ui;
}

void corrector::on_in_btn_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Input file"), "", tr("All Files (*)"));
    ui->in_file->setText(file);
}

void corrector::on_out_btn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/", QFileDialog::DontResolveSymlinks);
    ui->out_dir->setText(dir);
}

void corrector::setupGrades(){
    QStringList header;
    header<<"Grade"<<"Count"<<"Perc";
    ui->grade_results->setRowCount(5);
    ui->grade_results->setColumnCount(3);
    ui->grade_results->verticalHeader()->setVisible(true);
    ui->grade_results->verticalHeader()->setVisible(false);
    ui->grade_results->setHorizontalHeaderLabels(header);
    ui->grade_results->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->grade_results->setSelectionMode(QAbstractItemView::NoSelection);
    ui->grade_results->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui->grade_results->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->grade_results->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    int w = 0, h = 0;
    w += ui->grade_results->contentsMargins().left() + ui->grade_results->contentsMargins().right();
    h += ui->grade_results->contentsMargins().top() + ui->grade_results->contentsMargins().bottom();
    w += ui->grade_results->verticalHeader()->width();
    h += ui->grade_results->horizontalHeader()->height();

    for (int i=0; i<ui->grade_results->columnCount(); ++i)
        w += ui->grade_results->columnWidth(i);
    for (int i=0; i<ui->grade_results->rowCount(); ++i)
        h += ui->grade_results->rowHeight(i);

    ui->grade_results->setMinimumWidth(w-16);
    ui->grade_results->setMaximumWidth(w-16);
    ui->grade_results->setMinimumHeight(h);
    ui->grade_results->setMaximumHeight(h);
    ui->grade_results->setItem(0,0, new QTableWidgetItem("A"));
    ui->grade_results->setItem(1,0, new QTableWidgetItem("B"));
    ui->grade_results->setItem(2,0, new QTableWidgetItem("C"));
    ui->grade_results->setItem(3,0, new QTableWidgetItem("D"));
    ui->grade_results->setItem(4,0, new QTableWidgetItem("E"));
}

void corrector::on_run_clicked()
{
    if(!reuse){
        std::string file_path = ui->in_file->toPlainText().toStdString();
        if(file_path.empty()) return;
        an->annotate(file_path);
        reuse = true;
    }else{
        PRINT(an)
        an->annotate_algo();
    }
    auto results = an->getGradeResults();
    auto group_data = an->getGroupData();
    float sum = 0;
    for(auto& pair : results)
        sum += pair;

    sum = 1/sum;

    auto n_rows = ui->grade_results->rowCount();
    for (int row = 0; row < n_rows; row++) {
        ui->grade_results->setItem(row, 1, new QTableWidgetItem(QString::number(results[row])));
        ui->grade_results->setItem(row, 2, new QTableWidgetItem(QString::number(results[row]*sum)));
    }

    QSet<QString> set;
    for(auto& pair : group_data){
        set << QString::fromStdString(pair.first);
        for(auto& bin_name : pair.second.bins)
            set << QString::fromStdString(bin_name);

    }

    QCompleter * completer = new QCompleter(set.values(), this);
    ui->search_field->setCompleter(completer);
    if(graph) delete graph;
    graph = new GraphWidget( an, ui->graphicsView);
    graph->show();
}






void corrector::on_search_btn_clicked()
{
    QString name = ui->search_field->text();
    if(name.isEmpty()) return;
    graph->highlight(name);
}
