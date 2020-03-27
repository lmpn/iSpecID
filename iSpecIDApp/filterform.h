#ifndef FILTERFORM_H
#define FILTERFORM_H

#include <QDialog>

namespace Ui {
class FilterForm;
}

class FilterForm : public QDialog
{
    Q_OBJECT

public:
    explicit FilterForm(QWidget *parent = nullptr);
    ~FilterForm();
    bool getMatch();
    QString getBin();
    QString getSpecies();
    QString getInstitution();
    QString getGrade();
    bool isAccepted(){return accepted;}


private slots:
    void on_buttonBox_accepted();

private:
    Ui::FilterForm *ui;
    bool accepted = false;
};

#endif // FILTERFORM_H
