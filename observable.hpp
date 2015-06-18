#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP

#include <armadillo>
#include <QVector>
#include <QString>
#include <QColor>
#include <iostream>

#include "graph_data.hpp"
#include "qcustomplot.hpp"

static const QVector<QColor> RWTH_Colors = {
    {0, 84, 159},  // blau
    {204, 7, 30},  // rot
    {97, 33, 88},  // violett
    {87, 171, 39}  // gruen
};

class observable {
public:
    QString title;
    QString ylabel;

    QVector<double> x;
    QVector<double> t;

    virtual inline ~observable() {
    }

    virtual void setup(QCustomPlot & qcp) = 0;
    virtual void update(QCustomPlot & qcp, int m = 0) = 0;
};

// xobservable
// ---------------------------------------------------------------------------------------------------------------------------

class xobservable : public observable {
public:
    QVector<xgraph_data> data;

    inline xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t);
    inline void setup(QCustomPlot & qcp) override;
    inline void update(QCustomPlot & qcp, int m = 0) override;
    inline void add_data(const xgraph_data & multigraph_data);
};

xobservable::xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t) {
    this->title = title;
    this->ylabel = ylabel;
    this->x = x;
    this->t = t;
}

void xobservable::setup(QCustomPlot & qcp) {
    static const QString xlabel = "x / nm";

    qcp.clearGraphs();
    qcp.clearItems();

    qcp.xAxis->setRange(*(x.begin()), *(x.end() - 1)); // assume that x is ordered
    qcp.xAxis->setLabel(xlabel);

    double global_min = +1e200;
    double global_max = -1e200;
    for (int i = 0; i < data.size(); ++i) {
        qcp.addGraph();
        qcp.graph(i)->setName(data[i].title);
        qcp.graph(i)->setPen(QPen(RWTH_Colors[i]));
        global_min = (global_min < data[i].min) ? global_min : data[i].min;
        global_max = (global_max > data[i].max) ? global_max : data[i].max;
    }
    qcp.yAxis->setRange(global_min, global_max);
    qcp.yAxis->setLabel(ylabel);
}

void xobservable::update(QCustomPlot & qcp, int m) {
    for (int i = 0; i < data.size(); ++i) {
        qcp.graph(i)->setData(x, data[i].data[m]);
    }
    qcp.replot();
}

void xobservable::add_data(const xgraph_data & multigraph_data) {
    data.push_back(multigraph_data);
}

// tobservable
// ---------------------------------------------------------------------------------------------------------------------------

class tobservable : public observable {
public:
    QVector<tgraph_data> data;

    inline tobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t);
    inline void setup(QCustomPlot & qcp) override;
    inline void update(QCustomPlot & qcp, int m = 0) override;
    inline void add_data(const tgraph_data & graph_data);
};

tobservable::tobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t) {
    this->title = title;
    this->ylabel = ylabel;
    this->x = x;
    this->t = t;
}

void tobservable::setup(QCustomPlot & qcp) {
    static const QString xlabel = "t / s";

    qcp.clearGraphs();
    qcp.clearItems();

    qcp.xAxis->setRange(*(t.begin()), *(t.end() - 1)); // assume that t is ordered
    qcp.xAxis->setLabel(xlabel);

    double global_min = +1e200;
    double global_max = -1e200;
    for (int i = 0; i < data.size(); ++i) {
        qcp.addGraph();
        qcp.graph(i)->setName(data[i].title);
        qcp.graph(i)->setPen(QPen(RWTH_Colors[i]));
        global_min = (global_min < data[i].min) ? global_min : data[i].min;
        global_max = (global_max > data[i].max) ? global_max : data[i].max;

        // setup the data-tracers:
        data[i].tracer = new QCPItemTracer(&qcp);
        qcp.addItem(data[i].tracer);
        data[i].tracer->setGraph(qcp.graph(i));

        data[i].tracer->setInterpolating(true);
        data[i].tracer->setStyle(QCPItemTracer::tsCircle);
        data[i].tracer->setPen(QPen(Qt::red));
        data[i].tracer->setBrush(Qt::red);
        data[i].tracer->setSize(7);
    }
    qcp.yAxis->setRange(global_min, global_max);
    qcp.yAxis->setLabel(ylabel);

}

void tobservable::update(QCustomPlot & qcp, int m) {
    for (int i = 0; i < data.size(); ++i) {
        qcp.graph(i)->setData(t, data[i].data);
        data[i].tracer->setGraphKey(t[m]);
    }
    qcp.replot();
}

void tobservable::add_data(const tgraph_data & graph_data) {
    data.push_back(graph_data);
}

#endif
