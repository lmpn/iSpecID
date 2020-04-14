#ifndef FILTEROP_H
#define FILTEROP_H
#include <QWidget>
#include <QHBoxLayout>
#include <QRadioButton>
#include <functional>
class FilterOp : public QWidget
{
    Q_OBJECT
    QHBoxLayout *hlayout;
    QRadioButton * andRadioBtn;
    QRadioButton * orRadioBtn;
    template<class T>
    using Func = std::function<bool(T)>;
public:
   explicit FilterOp(QWidget *parent = Q_NULLPTR) :QWidget(parent){

        hlayout = new QHBoxLayout(this);
        hlayout->setObjectName(QString::fromUtf8("hlayout"));
        andRadioBtn = new QRadioButton(this);
        andRadioBtn->setObjectName(QString::fromUtf8("andRadioButton"));
        andRadioBtn->setText("And");
        andRadioBtn->setChecked(true);
        hlayout->addWidget(andRadioBtn);

        orRadioBtn = new QRadioButton(this);
        orRadioBtn->setObjectName(QString::fromUtf8("radioButton_2"));
        orRadioBtn->setText("Or");

        hlayout->addWidget(orRadioBtn);

    }

    int andOr(){
        return andRadioBtn->isChecked();
    }

    template<class T>
    Func<T> operate(Func<T> left, Func<T> right){
        auto op = andRadioBtn->isChecked() ? std::logical_and<Func<T>>() : std::logical_or<Func<T>>();
        Func<T> fn = [left, right, op](T item){
            return op(left(item), right(item));
        };
        return fn;
    }

};
#endif // FILTEROP_H
