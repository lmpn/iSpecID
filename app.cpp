#include "iSpecIDApp/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString db_path = a.applicationDirPath() + QDir::separator() + "ispecid.sqlite";
    MainWindow w;
    w.show();
    return a.exec();
}
