#include "filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QStringList header, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilterDialog)
{
    ui->setupUi(this);
    this->header = header;
    ui->horizontalLayout->setAlignment(Qt::AlignHCenter);
    connect(ui->addBtn, &QPushButton::clicked,
            ui->fsarea,[this](){ui->fsarea->addWidget(this->header);});
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
