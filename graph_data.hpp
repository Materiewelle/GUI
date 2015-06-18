#ifndef GRAPH_DATA_HPP
#define GRAPH_DATA_HPP

#include <QVector>
#include <QString>

#include <qcustomplot.hpp>

class graph_data {
public:
    Qstring title;
    double min;
    double max;
};

class xgraph_data : public graph_data {
public:
    QVector<QVector<double>> data;

    inline xgraph_data(const QString & title, const QVector<QVector<double>> & data, double min, double max)
        : graph_data{title, min, max}, this->data(data) {
    }
};

class tgraph_data : public graph_data {
public:
    QVector<double> data;
    QCPItemTracer * tracer;

    inline tgraph_data(const QString & title, const QVector<double> & data, double min, double max)
        : graph_data{title, min, max}, this->data(data) , tracer(nullptr) {
    }
};

#endif

