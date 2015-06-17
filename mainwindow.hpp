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
//#include "device.hpp"

class observable {
public:
    QString title;
    QString xlabel;
    QString ylabel;

    std::string folder;
    std::string file;

    inline virtual void configure_plot(QCustomPlot * qcp) = 0;
    inline virtual void update_plot(QCustomPlot * qcp, int m = 0) = 0;

    inline observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_) :
        title(title_), xlabel(xlabel_), ylabel(ylabel_), folder(folder_), file(file_) {
    }
};

class moving_graph_observable : public observable {
public:
    arma::mat data;
    arma::vec x;
    QVector<double> qx;

    inline void configure_plot(QCustomPlot * qcp) {
        qcp->xAxis->setLabel(xlabel);
        qcp->yAxis->setLabel(ylabel);
        double x0 = arma::min(arma::min(x));
        double x1 = arma::max(arma::max(x));
        qcp->xAxis->setRange(x0, x1);
        double y0 = arma::min(arma::min(data));
        double y1 = arma::max(arma::max(data));
        qcp->yAxis->setRange(y0, y1);
    }

    inline moving_graph_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_)
        : observable(title_, xlabel_, ylabel_, folder_, file_) {
        data.load(folder + "/" + file);
        x.load(folder + "/xtics.arma");
        qx = QVector<double>(x.size());
        std::copy(x.colptr(0), x.colptr(0) + x.size(), qx.data());
    }
};

class single_moving_graph_observable : public moving_graph_observable {
public:
    QVector<double> qy;

    inline void update_plot(QCustomPlot * qcp, int m = 0) {
        std::copy(data.colptr(m), data.colptr(m) + x.size(), qy.data());
        qcp->addGraph();
        qcp->graph(0)->setData(qx, qy);
        qcp->replot();
    }

    inline single_moving_graph_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_)
        : moving_graph_observable(title_, xlabel_, ylabel_, folder_, file_) {
    }
};

//class static_graph_observable : public observable {

//};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    inline MainWindow(const std::string folder);

private slots:
    inline void on_scroll_sliderMoved();
    inline void on_selection_currentIndexChanged(int index);

private:
    Ui::MainWindow ui;
    std::string folder;
//    device d;
    arma::vec t;
    std::vector<single_moving_graph_observable> obs;

    inline void update();
    inline void setup_observables();
};

MainWindow::MainWindow(std::string datafolder) :
folder(datafolder) {
    using namespace std::string_literals;

    t.load(folder + "/ttics.arma"s);

    ui.setupUi(this);
    setWindowTitle("Time dependent observables");

    // allow dragging and zoom
    ui.plot->setInteraction(QCP::iRangeDrag, true);
    ui.plot->setInteraction(QCP::iRangeZoom, true);

    setup_observables();

    // add entries in dropdown-menu
    for (int i = 0; i < obs.size(); ++i) {
        ui.selection->addItem(obs[i].title);
    }
    ui.selection->setCurrentIndex(0); // implied update()
}

void MainWindow::setup_observables() {;

//    obs.push_back(bandstructure_observable{"Potential", "x / nm", "psi / V", folder, "phi.arma"s});
    obs.push_back(single_moving_graph_observable{"Potential", "x / nm", "psi / V", folder, "phi.arma"});
    obs.push_back(single_moving_graph_observable{"Charge density", "x / nm", "n / C m^-3", folder, "n.arma"});
    obs.push_back(single_moving_graph_observable{"Current (spacial)", "x / nm", "I / A", folder, "I.arma"});
}

void MainWindow::update() {
    int m = ui.scroll->value() * t.size() / (ui.scroll->maximum() + 1);
    int s = ui.selection->currentIndex();

    // let the observable update the plot
    obs[s].update_plot(ui.plot, m);

    // update the time-display
    QString qs = "t = ";
    QTextStream qts(&qs);
    qts.setRealNumberNotation(QTextStream::FixedNotation);
    qts.setRealNumberPrecision(5);
    qts << t(m) * 1e12 << " ps";
    ui.label->setText(qs);
}

void MainWindow::on_scroll_sliderMoved() {
    update();
}

void MainWindow::on_selection_currentIndexChanged(int index) {
    obs[index].configure_plot(ui.plot);
    update();
}

#endif // MAINWINDOW_HPP
