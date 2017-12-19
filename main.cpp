#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Cyril Margorin");
    a.setOrganizationDomain("tower.pp.ru");
    a.setApplicationName("ParallelFileProcessor");
    a.setApplicationDisplayName(QCoreApplication::translate("GLOBAL", "Parallel file processor"));
    MainWindow w;
    w.show();

    return a.exec();
}
