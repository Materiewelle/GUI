#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP

#include <armadillo>
#include <QVector>
#include <QString>
#include <iostream>

#include "qcustomplot.hpp"


class observable {
public:
    QString title;
    QString ylabel;

    QVector<double> x, t;

    virtual void setup(QCustomPlot * qcp) = 0;
    virtual void update(QCustomPlot * qcp, int m = 0) = 0;
};

// xobservable
// ---------------------------------------------------------------------------------------------------------------------------

class xobservable : public observable {
public:
    static const QString xlabel = "x / nm";

    QVector<xgraph_data> data;

    inline xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t);
    inline void setup(QCustomPlot * qcp) override;
    inline void update(QCustomPlot * qcp, int m = 0) override;
    inline void add_data(const xgraph_data & multigraph_data);
};

xobservable::xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t)
    : observable{title, ylabel, x, t} {
}

void xobservable::setup(QCustomPlot * qcp) {
    qcp->clearGraphs();

    qcp->xAxis->setRange(*(x.begin()), *(x.end())); // assume that x is ordered
    qcp->xAxis->setLabel(xlabel);

    double global_min = +1e318;
    double global_max = -1e318;
    for (int i = 0; i < data.size(); ++i) {
        qcp->addGraph();
        global_min = (global_min < data[i].min) ? global_min : data[i].min;
        global_max = (global_max > data[i].max) ? global_max : data[i].max;
    }
    qcp->yAxis->setRange(global_min, global_max);
    qcp->yAxis->setLabel(ylabel);
}

void xobservable::update(QCustomPlot * qcp, int m = 0) {
    for (int i = 0; i < data.size(); ++i) {
        qcp->graph(i)->setData(x, data.data[i][m]);
    }
    qcp->replot();
}

void xobservable::add_data(const xgraph_data & multigraph_data) {
    data.push_back(multigraph_data);
}

// tobservable
// ---------------------------------------------------------------------------------------------------------------------------

class tobservable : public observable {
public:
    static const QString xlabel = "t / s";

    QVector<tgraph_data> data;

    inline xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t);
    inline void setup(QCustomPlot * qcp) override;
    inline void update(QCustomPlot * qcp, int m = 0) override;
    inline void add_data(const tgraph_data & graph_data);
};

void tobservable::setup(QCustomPlot * qcp) {
    qcp->clearGraphs();

    qcp->xAxis->setRange(*(t.begin()), *(t.end())); // assume that t is ordered
    qcp->xAxis->setLabel(xlabel);

    double global_min = +1e318;
    double global_max = -1e318;
    for (int i = 0; i < data.size(); ++i) {
        qcp->addGraph();
        global_min = (global_min < data[i].min) ? global_min : data[i].min;
        global_max = (global_max > data[i].max) ? global_max : data[i].max;

        // setup the data-tracers:
        data[i].tracer = new QCPItemTracer(qcp);
        qcp->addItem(data[i].tracer);
        data[i].tracer->setGraph(qcp->graph(i));

//        data[i].tracer->setInterpolating(true);
//        data[i].tracer->setStyle(QCPItemTracer::tsCircle);
//        data[i].tracer->setPen(QPen(Qt::red));
//        data[i].tracer->setBrush(Qt::red);
//        data[i].tracer->tracer->setSize(7);
    }
    qcp->yAxis->setRange(global_min, global_max);
    qcp->yAxis->setLabel(ylabel);

}

void tobservable::update(QCustomPlot * qcp, int m = 0) {
    for (int i = 0; i < data.size(); ++i) {
        qcp->graph(i)->setData(t, data[i].data);
        data[i].tracer->setGraphKey(t[m]);
    }
    qcp->replot();
}

void tobservable::add_data(const xgraph_data & graph_data) {
    data.push_back(graph_data);
}

#endif
