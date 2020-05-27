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
    explicit ProjectSelectionDialog(QString app_dir,QString current_project, QStringList projects, QWidget *parent = nullptr);
    QString getProject();
    ~ProjectSelectionDialog();
public slots:
    void deleteProject();
private:
    Ui::ProjectSelectionDialog *ui;
    QString app_dir;
    QString current_project;
};

#endif // PROJECTSELECTIONDIALOG_H
