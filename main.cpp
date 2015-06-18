#include <QApplication>

#include "main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    main_window w;
    w.setWindowTitle("GUI");
    w.show();

    return app.exec();
}
