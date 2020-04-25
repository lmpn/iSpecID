#include "gradingoptionsdialog.h"
#include "ui_gradingoptionsdialog.h"
#include <QMessageBox>
#include <qdebug.h>

GradingOptionsDialog::GradingOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GradingOptionsDialog)
{
    ui->setupUi(this);
}


void showErrorMessage(QString error_name, QString error){
    QMessageBox msgBox;
    msgBox.setText(error_name);
    msgBox.setInformativeText(error);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

bool GradingOptionsDialog::handle_click(){
    double dist;
    int labs;
    int seqs;
    bool ok;
    dist = ui->max_dist_text->toPlainText().toDouble(&ok);
    if(!ok || dist > 2){
        showErrorMessage("Maximun distance error","Can't parse value or value greater than 2");
        return false;
    }
    labs= ui->min_lab_text->toPlainText().toInt(&ok);
    if(!ok || labs < 2){
        showErrorMessage("Minimum of laboratories deposited sequences error","Can't parse value or value less than 2");
        return false;
    }
    seqs= ui->min_sequences_text->toPlainText().toInt(&ok);
    if(!ok && seqs < 3){
        showErrorMessage("Sequences deposited or published independently error", "Can't parse value or value lest than 3");
        return false;
    }
    emit save_config(dist,labs,seqs);
    return true;
}


GradingOptionsDialog::~GradingOptionsDialog()
{
    delete ui;
}

void GradingOptionsDialog::on_save_options_button_clicked()
{
    handle_click();
}

void GradingOptionsDialog::on_ok_button_clicked()
{
    auto ok = handle_click();
    if(ok)
        this->close();
}
