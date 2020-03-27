#include "filterform.h"
#include "ui_filterform.h"
#include <iostream>

FilterForm::FilterForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterForm)
{
    ui->setupUi(this);
}

FilterForm::~FilterForm()
{
    delete ui;
}

void FilterForm::on_buttonBox_accepted()
{
    accepted = true;
}

QString FilterForm::getBin(){
    return ui->bin_text->toPlainText();
}
QString FilterForm::getSpecies(){
    return ui->species_text->toPlainText();
}
QString FilterForm::getGrade(){
    return ui->comboBox->currentText();
}
QString FilterForm::getInstitution(){
    return ui->institution_text->toPlainText();
}
bool FilterForm::getMatch()
{
    return ui->checkBox->isChecked();
}
