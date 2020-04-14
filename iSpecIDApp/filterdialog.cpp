#include "filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilterDialog)
{
    ui->setupUi(this);
    ui->horizontalLayout->setAlignment(Qt::AlignHCenter);
    connect(ui->addBtn, &QPushButton::clicked,
            ui->fsarea,[this](){ui->fsarea->addWidget();});
}







FilterDialog::~FilterDialog()
{
    delete ui;
}


void FilterDialog::on_buttonBox_rejected()
{
    this->close();
}

void FilterDialog::on_buttonBox_accepted()
{
}
