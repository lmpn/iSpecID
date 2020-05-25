#ifndef PROJECTSELECTIONDIALOG_H
#define PROJECTSELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectSelectionDialog;
}

class ProjectSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSelectionDialog(QStringList projects, QWidget *parent = nullptr);
    QString getProject();
    ~ProjectSelectionDialog();

private:
    Ui::ProjectSelectionDialog *ui;
};

#endif // PROJECTSELECTIONDIALOG_H
