#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include "iSpecIDApp/filterscrollarea.h"
#include "ui_filterdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FilterDialog; }
QT_END_NAMESPACE

class FilterDialog : public QDialog
{
    Q_OBJECT
    QStringList header;

public:
    FilterDialog(QStringList header, QList<QStringList> completions, QWidget *parent = nullptr);
    ~FilterDialog();
    template<class T>
    using Func = std::function<bool(T)>;
    template<class T>
    Func<T> getFilterFunc(){
        bool keep = ui->keepCheckBox->isChecked();
        auto func = fs->getFilterFunc<T>();
        if(keep){
            func = [func](T item) {return !func(item);};
        }
        return func;
    }


    bool accepted(){return ok;}

private slots:
    void onButtonboxRejected();
    void onButtonboxAccepted();

private:
    bool ok = false;
    FilterScrollArea *fs;
    Ui::FilterDialog *ui;

};
#endif // FILTERDIALOG_H
