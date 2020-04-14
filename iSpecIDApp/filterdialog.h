#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class FilterDialog; }
QT_END_NAMESPACE

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    FilterDialog(QWidget *parent = nullptr);
    ~FilterDialog();

private slots:
    void on_buttonBox_rejected();

    void on_buttonBox_accepted();

private:
    Ui::FilterDialog *ui;
};
#endif // FILTERDIALOG_H
