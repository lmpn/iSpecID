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
    bool handleClick();

signals:
    void saveConfig(double, int, int);
private slots:
    void onSaveOptionsButtonClicked();
    void onOkButtonClicked();

private:
    Ui::GradingOptionsDialog *ui;
};

#endif // GRADINGOPTIONSDIALOG_H
