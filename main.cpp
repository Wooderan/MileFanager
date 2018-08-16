#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/icons/title_icon.png"));
//    w.setWindowTitle("File manager");
    w.show();


    return a.exec();
}
