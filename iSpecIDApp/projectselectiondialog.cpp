#include "projectselectiondialog.h"
#include "ui_projectselectiondialog.h"
#include <QStringListModel>
#include <QPushButton>
#include <qdebug.h>

ProjectSelectionDialog::ProjectSelectionDialog(QStringList projects, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectSelectionDialog)
{
    ui->setupUi(this);
    QStringListModel *model = new QStringListModel();
    model->setStringList(projects);
    ui->projectList->setModel(model);
}

QString ProjectSelectionDialog::getProject(){
    auto selection = ui->projectList->selectionModel()->selectedIndexes();
    if(selection.size() == 0) return "";
    return selection[0].data(Qt::DisplayRole).toString();
}

ProjectSelectionDialog::~ProjectSelectionDialog()
{
    delete ui;
}
