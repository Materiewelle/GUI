#include <QApplication>
#include <string>
#include <iostream>
#include <QDir>

#include "mainwindow.hpp"
#include "qcustomplot.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w("/home/fabian/Master_Thesis/Code/build/GUI/Debug/testdata");
    w.show();
    return a.exec();

//    QApplication a(argc, argv);
//    QString s;
//    if (argc == 2) {
//        s = QString(argv[1]);
//        QDir dir(s);
//        if (dir.exists()) {
//            MainWindow w(s.toUtf8().constData());
//            w.show();
//            return a.exec();
//        } else {
//            std::cout << "given directory does not exist" << std::endl;
//            return 1;
//        }
//    } else {
//        std::cout << "expected 1 argument (data folder)" << std::endl;
//        return 1;
//    }
}
