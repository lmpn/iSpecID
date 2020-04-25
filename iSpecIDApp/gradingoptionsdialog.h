#ifndef GRADINGOPTIONSDIALOG_H
#define GRADINGOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class GradingOptionsDialog;
}

class GradingOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GradingOptionsDialog(QWidget *parent = nullptr);
    ~GradingOptionsDialog();
    bool handle_click();

signals:
    void save_config(double, int, int);
private slots:
    void on_save_options_button_clicked();
    void on_ok_button_clicked();

private:
    Ui::GradingOptionsDialog *ui;
};

#endif // GRADINGOPTIONSDIALOG_H
