#ifndef CORRECTOR_H
#define CORRECTOR_H

#include <QWidget>
#include <QFileDialog>
#include <QCompleter>

#include <iostream>
#include <algorithm>
#include "graphwidget.h"

namespace Ui {
class corrector;
}

class corrector : public QWidget
{
    Q_OBJECT

public:
    explicit corrector(QWidget *parent = nullptr);
    ~corrector();

private slots:
    void on_in_btn_clicked();
    void on_out_btn_clicked();

    void on_run_clicked();

    void on_search_btn_clicked();

private:
    void setupGrades();
    bool reuse = false;
    Ui::corrector *ui;
    Annotator *an;
    GraphWidget *graph = nullptr;
};

#endif // CORRECTOR_H
