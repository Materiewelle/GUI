#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP

#include <armadillo>
#include <QVector>
#include <QString>
#include <QTextStream>
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

    bool logscale = false;

    double global_min = +1e200;
    double global_max = -1e200;

    virtual inline ~observable() {
    }

    virtual void setup(QCustomPlot & plot) = 0;
    virtual void update(QCustomPlot & plot, int m = 0) = 0;
};

// xobservable
// ---------------------------------------------------------------------------------------------------------------------------

class xobservable : public observable {
public:
    QVector<xgraph_data> data;

    inline xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t, bool logscale = false);
    inline void setup(QCustomPlot & plot) override;
    inline void update(QCustomPlot & plot, int m = 0) override;
    inline void add_data(const xgraph_data & multigraph_data);
};

xobservable::xobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t, bool logscale) {
    this->title = title;
    this->ylabel = ylabel;
    this->x = x;
    this->t = t;
    this->logscale = logscale;
}

void xobservable::setup(QCustomPlot & plot) {
    static const QString xlabel = "x / nm";

    plot.clearGraphs();
    plot.clearItems();

    plot.xAxis->setRange(*(x.begin()), *(x.end() - 1)); // assume that x is ordered
    plot.xAxis->setLabel(xlabel);

    if (logscale) {
        plot.yAxis->setScaleType(QCPAxis::stLogarithmic);
    } else {
        plot.yAxis->setScaleType(QCPAxis::stLinear);
    }

    for (int i = 0; i < data.size(); ++i) {
        plot.addGraph();
        plot.graph(i)->setName(data[i].title);
        plot.graph(i)->setPen(QPen(RWTH_Colors[i]));
        global_min = (global_min < data[i].min) ? global_min : data[i].min;
        global_max = (global_max > data[i].max) ? global_max : data[i].max;
    }
    plot.yAxis->setRange(global_min, global_max);
    plot.yAxis->setLabel(ylabel);
}

void xobservable::update(QCustomPlot & plot, int m) {
    for (int i = 0; i < data.size(); ++i) {
        plot.graph(i)->setData(x, data[i].data[m]);
    }
    plot.replot();
}

void xobservable::add_data(const xgraph_data & multigraph_data) {
    data.push_back(multigraph_data);
}

// tobservable
// ---------------------------------------------------------------------------------------------------------------------------

class tobservable : public observable {
public:
    QVector<tgraph_data> data;

    inline tobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t, bool logscale = false);
    inline void setup(QCustomPlot & plot) override;
    inline void update(QCustomPlot & plot, int m = 0) override;
    inline void setup_tracer(int i);
    inline void update_tracer(int i, int m);
    inline void add_data(const tgraph_data & graph_data);
};

tobservable::tobservable(const QString & title, const QString & ylabel, const QVector<double> & x, const QVector<double> & t, bool logscale) {
    this->title = title;
    this->ylabel = ylabel;
    this->x = x;
    this->t = t;
    this->logscale = logscale;
}

void tobservable::setup(QCustomPlot & plot) {
    static const QString xlabel = "t / s";

    plot.clearGraphs();
    plot.clearItems();

    plot.xAxis->setRange(*(t.begin()), *(t.end() - 1)); // assume that t is ordered
    plot.xAxis->setLabel(xlabel);

    if (logscale) {
        plot.yAxis->setScaleType(QCPAxis::stLogarithmic);
    } else {
        plot.yAxis->setScaleType(QCPAxis::stLinear);
    }

    for (int i = 0; i < data.size(); ++i) {
        // create and customize the graph
        plot.addGraph();
        plot.graph(i)->setName(data[i].title);
        plot.graph(i)->setPen(QPen(RWTH_Colors[i]));
        global_min = (global_min < data[i].min) ? global_min : data[i].min;
        global_max = (global_max > data[i].max) ? global_max : data[i].max;

        // make new tracing items and let plot take ownership of them
        data[i].tracer = new QCPItemTracer(&plot);
        data[i].label  = new QCPItemText(&plot);
        data[i].arrow  = new QCPItemCurve(&plot);
        plot.addItem(data[i].tracer);
        plot.addItem(data[i].label);
        plot.addItem(data[i].arrow);
        data[i].tracer->setGraph(plot.graph(i));
        data[i].label->setBrush(QBrush(QColor(255,255,255,130))); //transparent white

        setup_tracer(i);
    }
    plot.yAxis->setRange(global_min, global_max);
    plot.yAxis->setLabel(ylabel);

}

void tobservable::update(QCustomPlot & plot, int m) {
    for (int i = 0; i < data.size(); ++i) {
        plot.graph(i)->setData(t, data[i].data);
        update_tracer(i, m);
    }
    plot.replot();
}

void tobservable::setup_tracer(int i) {
    // setup the tracer:
    data[i].tracer->setInterpolating(true);
    data[i].tracer->setStyle(QCPItemTracer::tsCircle);
    data[i].tracer->setPen(QPen(RWTH_Colors[i]));
    data[i].tracer->setBrush(RWTH_Colors[i]);
    data[i].tracer->setSize(7);

    // setup the label showing the current value:
    data[i].label->position->setType(QCPItemPosition::ptPlotCoords);
//    data[i].label->setTextAlignment(Qt::AlignCenter);

    // setup the arrow pointing from the label to the tracer
    data[i].arrow->startDir->setParentAnchor(data[i].arrow->start);
    data[i].arrow->end->setParentAnchor(data[i].tracer->position);
    data[i].arrow->endDir->setParentAnchor(data[i].arrow->end);
    data[i].arrow->setHead(QCPLineEnding::esSpikeArrow);
}

void tobservable::update_tracer(int i, int m) {
    data[i].tracer->setGraphKey(t[m]);

    double y = data[i].data[m]; // shortcut

    QString s(data[i].title);
    QTextStream ts(&s);
    ts.setRealNumberNotation(QTextStream::SmartNotation);
    ts.setRealNumberPrecision(4);
    ts << ":\n" << y;
    data[i].label->setText(s);

    double width = (*(t.end() - 1) - *(t.begin()));
    double height = (global_max - global_min);
    double labeldir_x = (m < t.size() / 2) ? 1 : -1;
    double labeldir_y = (y < global_min + 0.5 * height) ? 1 : -1;
    double labelpos_x = t[m] + 0.1 * labeldir_x * width;
    double labelpos_y = y + 0.1 * labeldir_y * height;
    data[i].label->position->setCoords(labelpos_x, labelpos_y);

    if (labeldir_x > 0) {
        data[i].label->setTextAlignment(Qt::AlignLeft);
        data[i].label->setPadding(QMargins(6, 0, 0, 0));
        data[i].arrow->start->setParentAnchor(data[i].label->left);
    } else {
        data[i].label->setTextAlignment(Qt::AlignRight);
        data[i].label->setPadding(QMargins(0, 0, 6, 0));
        data[i].arrow->start->setParentAnchor(data[i].label->right);
    }

    // make it curvy (relative to actual window-size in px would be better):
    data[i].arrow->endDir->setCoords(20 * labeldir_x, -20 * labeldir_y); // pointing up/down from tracer
    data[i].arrow->startDir->setCoords(-20 * labeldir_x, 0); // pointing left/right from label
    // a little bit of offset to the tracer
    data[i].arrow->end->setCoords(7 * labeldir_x, -7 * labeldir_y);
}

void tobservable::add_data(const tgraph_data & graph_data) {
    data.push_back(graph_data);
}

#endif
