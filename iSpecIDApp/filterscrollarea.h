#ifndef FILTERSCROLLAREA_H
#define FILTERSCROLLAREA_H
#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QComboBox>
#include <QCompleter>
#include <QTextEdit>
#include "filterItem.h"
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

    void set_header(QStringList header){
        this->header = header;
    }

    void set_completions(QList<QStringList> completions){
        this->completions = completions;
    }

    template<class T>
    Func<T> getFilterFunc(){
        auto count = vlayout->rowCount();
        if(count == 1){
            return [](T item){ return true;};
        }
        FilterItem *filter_item = static_cast<FilterItem*>(vlayout->itemAtPosition(1,0)->widget());
        auto col = convertToColumnName(filter_item->current_group());
        auto val = filter_item->current_val().toStdString();
        Func<T> fn = [col, val](T item){ return item[col] == val;};

        for (int i = 2; i < count; i+=2) {
            FilterItem *filter_item = static_cast<FilterItem*>(vlayout->itemAtPosition(i+1,0)->widget());
            auto col = convertToColumnName(filter_item->current_group());
            auto val = filter_item->current_val().toStdString();
            Func<T> fn_i = [col, val](T item){ return item[col] == val;};
            FilterOp * fo = static_cast<FilterOp*>(vlayout->itemAtPosition(i,0)->widget());
            fn = fo->operate(fn,fn_i);
        }
        return fn;
    }

public slots:
    void add_widget(){
        auto count = vlayout->rowCount();
        if(count > 1){
            vlayout->addWidget(new FilterOp(), count, 0);
            count++;
        }
        auto filter_item = new FilterItem(header, completions);
        vlayout->addWidget(filter_item, count, 0);
    }
private:
    std::string convertToColumnName(QString name){
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
    QList<QStringList> completions;
    QList<QString> header;
};

#endif // FILTERSCROLLAREA_H
