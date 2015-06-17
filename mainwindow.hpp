#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <armadillo>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

#include <qcustomplot.hpp>
#include "ui_mainwindow.h" // created at compiletime
#include "device.hpp"

class observable {
public:
    QString title;
    QString xlabel;
    QString ylabel;

    std::string folder;
    std::string file;

    arma::mat data;

    // the following are not initialized in the virtual class!
    arma::vec x;
    QVector<double> qx;

    inline virtual void configure_plot(QCustomPlot * qcp) {
        qcp->clearGraphs();
        qcp->xAxis->setLabel(xlabel);
        qcp->yAxis->setLabel(ylabel);
        double x0 = arma::min(arma::min(x));
        double x1 = arma::max(arma::max(x));
        qcp->xAxis->setRange(x0, x1);
    }

    virtual void update_plot(QCustomPlot * qcp, int m = 0) = 0;

    inline observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_) :
        title(title_), xlabel(xlabel_), ylabel(ylabel_), folder(folder_), file(file_) {
        data.load(folder + "/" + file);
    }
};

class moving_graph_observable : public observable {
public:
    inline moving_graph_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_)
        : observable(title_, xlabel_, ylabel_, folder_, file_) {
        x.load(folder + "/xtics.arma");
        qx = QVector<double>(x.size());
        std::copy(x.colptr(0), x.colptr(0) + x.size(), qx.data());
    }
};

class single_moving_graph_observable : public moving_graph_observable {
public:
    QVector<double> qy;

    inline void configure_plot(QCustomPlot * qcp) override {
        observable::configure_plot(qcp);
        double y0 = arma::min(arma::min(data));
        double y1 = arma::max(arma::max(data));
        qcp->yAxis->setRange(y0, y1);
        qcp->addGraph();
    }

    inline void update_plot(QCustomPlot * qcp, int m = 0) override {
        std::copy(data.colptr(m), data.colptr(m) + x.size(), qy.data());
        qcp->graph(0)->setData(qx, qy);
        qcp->replot();
    }

    inline single_moving_graph_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_)
        : moving_graph_observable(title_, xlabel_, ylabel_, folder_, file_) {
        qy = QVector<double>(x.size());
    }
};

class bandstructure_observable : public moving_graph_observable {
public:
    const device & d;
    QVector<double> qc, qv;

    inline void configure_plot(QCustomPlot * qcp) override {
        moving_graph_observable::configure_plot(qcp);
        double y0 = arma::min(arma::min(data)) - d.E_g / 2;
        double y1 = arma::max(arma::max(data)) + d.E_g / 2;
        qcp->yAxis->setRange(y0, y1);
        qcp->addGraph();
        qcp->addGraph();
    }

    inline void update_plot(QCustomPlot * qcp, int m = 0) override {
        arma::vec tmp_c = data.col(m) + d.E_g / 2;
        std::copy(tmp_c.memptr(), tmp_c.memptr() + x.size(), qc.data());
        qcp->graph(0)->setData(qx, qc);

        arma::vec tmp_v = data.col(m) - d.E_g / 2;
        std::copy(tmp_v.memptr(), tmp_v.memptr() + x.size(), qv.data());
        qcp->graph(1)->setData(qx, qv);

        qcp->replot();
    }

    inline bandstructure_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_, const device & d_)
        : moving_graph_observable(title_, xlabel_, ylabel_, folder_, file_), d(d_) {
        qc = QVector<double>(x.size());
        qv = QVector<double>(x.size());
    }
};

class static_graph_observable : public observable {
public:
    inline static_graph_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_)
        : observable(title_, xlabel_, ylabel_, folder_, file_) {
        x.load(folder + "/ttics.arma");
        qx = QVector<double>(x.size());
        std::copy(x.colptr(0), x.colptr(0) + x.size(), qx.data());
    }
};

class single_static_graph_observable : public static_graph_observable {
public:
    arma::vec y;
    QVector<double> qy;
    std::unique_ptr<QCPItemTracer> tracer;

    inline void configure_plot(QCustomPlot * qcp) override {
        observable::configure_plot(qcp);
        double y0 = arma::min(y);
        double y1 = arma::max(y);
        qcp->yAxis->setRange(y0, y1);
        qcp->addGraph();
        qcp->graph(0)->addData(qx, qy);

        tracer = std::make_unique<QCPItemTracer>(qcp);
        qcp->addItem(tracer.get());
        tracer->setGraph(qcp->graph(0));
        tracer->setGraphKey(qx[0]);
        tracer->setInterpolating(true);
        tracer->setStyle(QCPItemTracer::tsCircle);
        tracer->setPen(QPen(Qt::red));
        tracer->setBrush(Qt::red);
        tracer->setSize(7);
    }

    inline void update_plot(QCustomPlot * qcp, int m = 0) override {
        tracer->setGraphKey(qx[m]);
        qcp->replot();
    }

    inline single_static_graph_observable(QString title_, QString xlabel_, QString ylabel_, std::string folder_, std::string file_)
        : static_graph_observable(title_, xlabel_, ylabel_, folder_, file_) {
        y = data.col(1);
        qy = QVector<double>(x.size());
        std::copy(y.memptr(), y.memptr() + x.size(), qy.data());
    }
};

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
    device d;
    arma::vec t;
    std::vector<std::unique_ptr<observable>> obs;

    inline void update();
    inline void setup_observables();
};

MainWindow::MainWindow(std::string datafolder)
    : folder(datafolder) {
    using namespace std::string_literals;

    std::ifstream device_params(folder + "/device.ini");
    std::string device_string((std::istreambuf_iterator<char>(device_params)),
                               std::istreambuf_iterator<char>());
    d = device(device_string);

    t.load(folder + "/ttics.arma"s);

    ui.setupUi(this);
    setWindowTitle("Time dependent observables");

    // allow dragging and zoom
    ui.plot->setInteraction(QCP::iRangeDrag, true);
    ui.plot->setInteraction(QCP::iRangeZoom, true);

    setup_observables();

    // add entries in dropdown-menu
    for (unsigned i = 0; i < obs.size(); ++i) {
        ui.selection->addItem(obs[i]->title);
    }
    ui.selection->setCurrentIndex(0); // implied update()
}

void MainWindow::setup_observables() {
    using namespace std;

    obs.push_back(make_unique<bandstructure_observable>("Bandstructure", "x / nm", "phi / V", folder, "phi.arma"s, d));
//    obs.push_back(make_unique<single_moving_graph_observable>("Potential", "x / nm", "E_c, E_v / V", folder, "phi.arma"));
    obs.push_back(make_unique<single_moving_graph_observable>("Charge density", "x / nm", "n / C m^-3", folder, "n.arma"));
    obs.push_back(make_unique<single_moving_graph_observable>("Current (spacial)", "x / nm", "I / A", folder, "I.arma"));
    obs.push_back(make_unique<single_static_graph_observable>("Gate voltage", "t / s", "V_d / V", folder, "V.arma"));
}

void MainWindow::update() {
    int m = ui.scroll->value() * t.size() / (ui.scroll->maximum() + 1);
    int s = ui.selection->currentIndex();

    // let the observable update the plot
    obs[s]->update_plot(ui.plot, m);

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
    obs[index]->configure_plot(ui.plot);
    update();
}

#endif // MAINWINDOW_HPP
