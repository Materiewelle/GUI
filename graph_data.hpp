#ifndef GRAPH_DATA_HPP
#define GRAPH_DATA_HPP

#include <QVector>
#include <QString>

#include <qcustomplot.hpp>

// Theese are just some POD-classes which are used by the "observable"-class

class graph_data {
public:
    QString title;
    double min;
    double max;
};

class xgraph_data : public graph_data {
public:
    QVector<QVector<double>> data; // a vector of graph-data (one element per timestep)

    inline xgraph_data() {
    }
    inline xgraph_data(const QString & title, const QVector<QVector<double>> & data_, double min, double max)
        : graph_data{title, min, max}, data(data_) {
    }
};

class tgraph_data : public graph_data {
public:
    QVector<double> data; // the same graph for every timestep
    QCPItemTracer * tracer; // a little red dot that indicates the current time
    QCPItemText * label; // indicates the current value
    QCPItemCurve * arrow; // pointing from label to tracer

    inline tgraph_data() {
    }
    inline tgraph_data(const QString & title, const QVector<double> & data_, double min, double max)
        : graph_data{title, min, max}, data(data_) , tracer(nullptr), label(nullptr), arrow(nullptr) {
    }
};

#endif

