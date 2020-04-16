#ifndef FILTERSCROLLAREA_H
#define FILTERSCROLLAREA_H
#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QComboBox>
#include <QTextEdit>
#include "filterOp.h"
#include <qdebug.h>


class FilterScrollArea : public QScrollArea
{
    QWidget *contentWidget;
    QGridLayout *vlayout;
    template<class T>
    using Func = std::function<bool(T)>;
public:
    FilterScrollArea(QWidget *parent = Q_NULLPTR) :QScrollArea(parent){
        setWidgetResizable(true);
        contentWidget = new QWidget(this);
        setWidget(contentWidget);
        vlayout = new QGridLayout(contentWidget);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    template<class T>
    Func<T> getFilterFunc(){
        auto count = vlayout->rowCount();
        if(count == 1){
            return [](T item){ return true;};
        }

        QComboBox *qbl = static_cast<QComboBox*>(vlayout->itemAtPosition(1,0)->widget());
        QTextEdit *tel = static_cast<QTextEdit*>(vlayout->itemAtPosition(1,1)->widget());
        auto col = convertToColumnName(qbl->currentText().toStdString());
        auto val = tel->toPlainText().toStdString();
        Func<T> fn = [col, val](T item){ return item[col] == val;};

        for (int i = 3; i < count; i+=2) {
            QComboBox *qbr = static_cast<QComboBox*>(vlayout->itemAtPosition(i,0)->widget());
            QTextEdit *ter = static_cast<QTextEdit*>(vlayout->itemAtPosition(i,1)->widget());
            auto col = convertToColumnName(qbr->currentText().toStdString());
            auto val = ter->toPlainText().toStdString();
            Func<T> fn_i = [col, val](T item){ return item[col] == val;};
            FilterOp * fo = static_cast<FilterOp*>(vlayout->itemAtPosition(i-1,1)->widget());
            fn = fo->operate(fn,fn_i);
        }
        return fn;
    }

public slots:

    void addWidget(QStringList header){
        auto count = vlayout->rowCount();
        if(count > 1){
            vlayout->addWidget(new FilterOp(), count, 1);
            count++;
        }
        auto filterComboBox = new QComboBox();
        filterComboBox->addItems(header);
        auto filterCondition = new QTextEdit();
        filterCondition->setMaximumSize(QSize(16777215, 25));
        filterCondition->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        vlayout->addWidget(filterComboBox, count, 0);
        vlayout->addWidget(filterCondition, count, 1);
    }
private:
    std::string convertToColumnName(std::string name){
        if(name == "Species"){
            return "species_name";
        }
        if(name == "Bin"){
            return "bin_uri";
        }
        if(name == "Institution"){
            return "institution_storing";
        }
        if(name == "Grade"){
            return "grade";
        }
        return "";
    }
};

#endif // FILTERSCROLLAREA_H
