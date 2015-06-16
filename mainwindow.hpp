#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <armadillo>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

#include <qcustomplot.hpp>
#include "ui_mainwindow.h" // created at compiletime

enum {
    phi = 0,
    n = 1,
    I = 2
};

class observable {
public:
    QString title;
    QString xlabel;
    QString ylabel;

    bool logscale;

    std::string filename;
    arma::mat data;

    inline observable() {
    }

    inline observable(QString _title, QString _xlabel, QString _ylabel, bool _logscale, std::string _filename) :
        title(_title), xlabel(_xlabel), ylabel(_ylabel), logscale(_logscale), filename(_filename) {
        data.load(filename);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    inline MainWindow(const std::string folder);

    inline void setup_observables();
    inline void update();

    std::vector<observable> obs;

    arma::vec x, t;
    QVector<double> qx, qy;

private slots:
    inline void on_scroll_sliderMoved();
    inline void on_selection_currentIndexChanged(int index);

private:
    Ui::MainWindow ui;
    const std::string folder;
};

MainWindow::MainWindow(const std::string datafolder) :
folder(datafolder) {
    using namespace std::string_literals;

    ui.setupUi(this);
    setWindowTitle("Time dependent observables");

    x.load(folder + "/xtics.arma"s);
    t.load(folder + "/ttics.arma"s);
    double x0 = arma::min(x);
    double x1 = arma::max(x);
    ui.plot->xAxis->setRange(x0, x1);

    qx = QVector<double>(x.size());
    std::copy(x.colptr(0), x.colptr(0) + x.size(), qx.data());
    qy = QVector<double>(x.size());

    setup_observables();

    // add entries in dropdown-menu
    for (auto it = obs.begin(); it != obs.end(); ++it) {
        ui.selection->addItem(it->title);
    }
    ui.selection->setCurrentIndex(0); // implied update()
}

void MainWindow::setup_observables() {
    using namespace std::string_literals;

    obs = std::vector<observable>(3);
    obs[phi] = observable{"Potential", "x / nm", "phi / V", false, folder + "/phi.arma"s};
    obs[n] = observable{"Charge density", "x / nm", "n / C m^{-3}", false, folder + "/n.arma"s};
    obs[I] = observable{"Current (spacial)", "x / nm", "I / A", false, folder + "/I.arma"s};
}

void MainWindow::update() {
    int m = ui.scroll->value() * t.size() / 100;
    int s = ui.selection->currentIndex();

    // copy selected data from arma::mat to QVector
    std::copy(obs[s].data.colptr(m), obs[s].data.colptr(m) + x.size(), qy.data());
    ui.plot->addGraph(); // seems like this needs to be done every time
    ui.plot->graph(0)->setData(qx, qy);
    ui.plot->replot();
}

void MainWindow::on_scroll_sliderMoved() {
    update();
}

void MainWindow::on_selection_currentIndexChanged(int index) {
    ui.plot->xAxis->setLabel(obs[index].xlabel);
    ui.plot->yAxis->setLabel(obs[index].ylabel);

    double y0 = arma::min(arma::min(obs[index].data));
    double y1 = arma::max(arma::max(obs[index].data));

    if (obs[index].logscale) {
        ui.plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        y0 = std::log10(y0);
        y1 = std::log10(y1);
    } else {
        ui.plot->yAxis->setScaleType(QCPAxis::stLinear);
    }
    ui.plot->yAxis->setRange(y0, y1);

    update();
}

#endif // MAINWINDOW_HPP
