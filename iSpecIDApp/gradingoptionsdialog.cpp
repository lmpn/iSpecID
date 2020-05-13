#include "gradingoptionsdialog.h"
#include "ui_gradingoptionsdialog.h"
#include <QMessageBox>
#include <qdebug.h>

GradingOptionsDialog::GradingOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GradingOptionsDialog)
{
    ui->setupUi(this);
    connect(ui->save_options_button, SIGNAL(clicked()),
            this, SLOT(onSaveOptionsButtonClicked()));
    connect(ui->ok_button, SIGNAL(clicked()),
            this, SLOT(onOkButtonClicked()));
}


void showErrorMessage(QString error_name, QString error){
    QMessageBox::critical(nullptr, error_name, error, QMessageBox::Cancel);
}

bool GradingOptionsDialog::handleClick(){
    double dist;
    int labs;
    int seqs;
    bool ok;
    dist = ui->max_dist_text->text().toDouble(&ok);
    if(!ok || dist > 2){
        showErrorMessage("Maximun distance error","Can't parse maximun distance or value greater than 2");
        return false;
    }
    labs= ui->min_lab_text->text().toInt(&ok);
    if(!ok || labs < 2){
        showErrorMessage("Minimum of laboratories deposited sequences error","Can't parse minimum number of laboratories deposited sequences or value less than 2");
        return false;
    }
    seqs= ui->min_sequences_text->text().toInt(&ok);
    if(!ok && seqs < 3){
        showErrorMessage("Sequences deposited or published independently error", "Can't parse minimun number of sequences deposited or published independently or value less than 3");
        return false;
    }
    emit saveConfig(dist,labs,seqs);
    return true;
}


GradingOptionsDialog::~GradingOptionsDialog()
{
    delete ui;
}

void GradingOptionsDialog::onSaveOptionsButtonClicked()
{
    handleClick();
}

void GradingOptionsDialog::onOkButtonClicked()
{
    auto ok = handleClick();
    if(ok)
        this->close();
}
