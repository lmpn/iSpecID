#include "filterdialog.h"

FilterDialog::FilterDialog(QStringList header, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilterDialog)
{
    ui->setupUi(this);
    this->header = header;
    fs = ui->fsarea;
    ui->horizontalLayout->setAlignment(Qt::AlignHCenter);
    connect(ui->addBtn, &QPushButton::clicked,
            fs,[this](){fs->add_widget(this->header);});
}


FilterDialog::~FilterDialog()
{
    delete ui;
}


void FilterDialog::on_buttonBox_rejected()
{
    close();
}

void FilterDialog::on_buttonBox_accepted()
{
    ok = true;
    close();
}
