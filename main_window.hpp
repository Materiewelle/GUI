#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <armadillo>
#include <memory>
#include <vector>

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QWidget>

#include "device.hpp"
#include "qcustomplot.hpp"
#include "observable.hpp"

class main_window : public QWidget
{
    Q_OBJECT

public:
    inline main_window(QWidget * parent = nullptr);

private slots:
    inline void load_data();
    inline void select_observable(int index);
    inline void set_time(int val);

private:
    QGridLayout layout;
    QPushButton open_button;
    QComboBox selection_box;
    QLabel time_label;
    QCustomPlot plot;
    QScrollBar time_scrollbar;

    QVector<double> x;
    QVector<double> t;

    int time_index;

    std::vector<std::unique_ptr<observable>> observables;
};

//----------------------------------------------------------------------------------------------------------------------

main_window::main_window(QWidget * parent)
    : QWidget(parent), time_index(0) {

    resize(800, 600);

    layout.addWidget(&open_button, 0, 0);
    layout.addWidget(&selection_box, 0, 1);
    layout.addWidget(&time_label, 0, 2);
    layout.addWidget(&plot, 1, 0, 1, 3);
    layout.addWidget(&time_scrollbar, 2, 0, 1, 3);
    setLayout(&layout);

    open_button.setText("Open Directory");

    selection_box.setEnabled(false);

    plot.setInteraction(QCP::iRangeDrag, true);
    plot.setInteraction(QCP::iRangeZoom, true);

    time_scrollbar.setOrientation(Qt::Horizontal);
    time_scrollbar.setTracking(true);
    time_scrollbar.setRange(0, 9999);
    time_scrollbar.setValue(0);
    time_scrollbar.setEnabled(false);

    QObject::connect(&open_button, SIGNAL(clicked()), this, SLOT(load_data()));
    QObject::connect(&selection_box, SIGNAL(currentIndexChanged(int)), this, SLOT(select_observable(int)));
    QObject::connect(&time_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(set_time(int)));
}

void main_window::load_data() {
    // clear old data
    observables.clear();

    plot.clearGraphs();
    time_scrollbar.setValue(0);
    time_scrollbar.setEnabled(false);
    selection_box.clear();
    selection_box.setCurrentIndex(0);
    selection_box.setEnabled(false);

    // open dialog
    QString dir = QFileDialog::getExistingDirectory(this, "Open Directory", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // load device.ini
    QFile device_file(dir + "/device.ini");
    if (!device_file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }
    QTextStream device_stream(&device_file);
    QString device_string = device_stream.readAll();
    device d(device_string.toStdString());

    auto load_1D = [] (const QString & file_name, QVector<double> & vec, double & min, double & max) -> bool {
        arma::vec av;
        if (!av.quiet_load(file_name.toStdString())) {
            return false;
        }

        min = av.min();
        max = av.max();
        double delta = max - min;
        min = min - delta * 0.05;
        max = max + delta * 0.05;

        vec = QVector<double>(av.size());
        std::copy(av.memptr(), av.memptr() + av.size(), vec.data());

        return true;
    };

    auto load_2D = [] (const QString & file_name, QVector<QVector<double>> & mat, double & min, double & max) -> bool {
        arma::mat am;
        if (!am.quiet_load(file_name.toStdString())) {
            return false;
        }

        min = am.min();
        max = am.max();
        double delta = max - min;
        min = min - delta * 0.05;
        max = max + delta * 0.05;

        mat = QVector<QVector<double>>(am.n_cols);

        for (unsigned i = 0; i < am.n_cols; ++i) {
            mat[i] = QVector<double>(am.n_rows);

            std::copy(am.colptr(i), am.colptr(i) + am.n_rows, mat[i].data());
        }

        return true;
    };

    // load xtics.arma, ttics.arma
    double xmin, xmax, tmin, tmax;
    if (!load_1D(dir + "/xtics.arma", x, xmin, xmax)) {
        return;
    }
    if (!load_1D(dir + "/ttics.arma", t, tmin, tmax)) {
        return;
    }

    // load phi.arma
    QVector<QVector<double>> phi;
    double phimin, phimax;
    if (load_2D(dir + "/phi.arma", phi, phimin, phimax)) {
        QVector<QVector<double>> vband(phi.size());
        QVector<QVector<double>> cband(phi.size());

        double vbandmin = phimin - 0.5 * (std::max(d.E_gc, d.E_g));
        double vbandmax = phimax - 0.5 * (std::min(d.E_gc, d.E_g));
        double cbandmin = phimin + 0.5 * (std::min(d.E_gc, d.E_g));
        double cbandmax = phimax + 0.5 * (std::max(d.E_gc, d.E_g));

        bool ok = true;
        for (int i = 0; i < phi.size(); ++i) {
            if (phi[i].size() != d.N_x) {
                ok = false;
                break;
            }
            vband[i] = QVector<double>(phi[i].size());
            cband[i] = QVector<double>(phi[i].size());
            for (int j = 0; j < d.N_sc; ++j) {
                vband[i][j] = phi[i][j] - 0.5 * d.E_gc;
                cband[i][j] = phi[i][j] + 0.5 * d.E_gc;
            }
            for (int j = d.N_sc; j < d.N_x - d.N_dc; ++j) {
                vband[i][j] = phi[i][j] - 0.5 * d.E_g;
                cband[i][j] = phi[i][j] + 0.5 * d.E_g;
            }
            for (int j = d.N_x - d.N_dc; j < d.N_x; ++j) {
                vband[i][j] = phi[i][j] - 0.5 * d.E_gc;
                cband[i][j] = phi[i][j] + 0.5 * d.E_gc;
            }
        }

        if (ok) {
            xobservable * bandstructure = new xobservable("Bandstructure", "phi / V", x, t);
            bandstructure->add_data({ "Valence Band", vband, vbandmin, vbandmax });
            bandstructure->add_data({ "Conduction Band", cband, cbandmin, cbandmax });
            observables.push_back(std::move(std::unique_ptr<xobservable>(bandstructure)));
        }
    }

    // load n.arma
    QVector<QVector<double>> n;
    double nmin, nmax;
    if (load_2D(dir + "/n.arma", n, nmin, nmax)) {
        xobservable * charge_density = new xobservable("Charge density", "n / C m^-3", x, t);
        charge_density->add_data({ "Charge density", n, nmin, nmax });
        observables.push_back(std::move(std::unique_ptr<observable>(charge_density)));
    }

    // load I.arma
    QVector<QVector<double>> I;
    double Imin, Imax;
    if (load_2D(dir + "/I.arma", I, Imin, Imax)) {
        xobservable * current = new xobservable("Current (spatial)", "I / A", x, t);
        current->add_data({ "Current", I, Imin, Imax });
        observables.push_back(std::move(std::unique_ptr<observable>(current)));

        QVector<double> I_s(I.size());
        QVector<double> I_d(I.size());
        double Ismin = Imax, Ismax = Imin, Idmin = Imax, Idmax = Imin;
        for (int i = 0; i < I.size(); ++i) {
            I_s[i] = I[i][0];
            I_d[i] = I[i][I[i].size() - 1];
            if (I_s[i] < Ismin) {
                Ismin = I_s[i];
            }
            if (I_s[i] > Ismax) {
                Ismax = I_s[i];
            }
            if (I_d[i] < Idmin) {
                Idmin = I_d[i];
            }
            if (I_d[i] > Idmax) {
                Idmax = I_d[i];
            }
        }

        double delta = Ismax - Ismin;
        Ismin = Ismin - delta * 0.05;
        Ismax = Ismax + delta * 0.05;

        delta = Idmax - Idmin;
        Idmin = Idmin - delta * 0.05;
        Idmax = Idmax + delta * 0.05;

        tobservable * current_s = new tobservable("Source Current", "I / A", x, t);
        current_s->add_data({ "Source Current", I_s, Ismin, Ismax });
        observables.push_back(std::move(std::unique_ptr<observable>(current_s)));

        tobservable * current_d = new tobservable("Drain Current", "I / A", x, t);
        current_d->add_data({ "Drain Current", I_d, Idmin, Idmax });
        observables.push_back(std::move(std::unique_ptr<observable>(current_d)));
    }

    // load V.arma
    QVector<QVector<double>> V;
    double Vmin, Vmax;
    if (load_2D(dir + "/V.arma", V, Vmin, Vmax)) {
        if (V.size() == 3) {
            tobservable * voltage = new tobservable("Voltage", "V / V", x, t);
            voltage->add_data({ "V_s", V[0], Vmin, Vmax });
            voltage->add_data({ "V_g", V[1], Vmin, Vmax });
            voltage->add_data({ "V_d", V[2], Vmin, Vmax });
            observables.push_back(std::move(std::unique_ptr<observable>(voltage)));
        }
    }

    // set time to 0
    time_scrollbar.setEnabled(true);
    time_scrollbar.setValue(0); // calls set_time

    // add entries in dropdown-menu
    selection_box.setEnabled(true);
    selection_box.clear();
    for (unsigned i = 0; i < observables.size(); ++i) {
        selection_box.addItem(observables[i]->title);
    }
    selection_box.setCurrentIndex(0); // calls select_observable(int)
}

void main_window::select_observable(int index) {
    if ((unsigned)index < observables.size()) {
        observables[index]->setup(plot);
        observables[index]->update(plot, time_index);
    }
}

void main_window::set_time(int val) {
    time_index = val * t.size() / (time_scrollbar.maximum() + 1);

    // update the time_label
    QString qs = "t = ";
    QTextStream qts(&qs);
    qts.setRealNumberNotation(QTextStream::FixedNotation);
    qts.setRealNumberPrecision(5);
    qts << t[time_index] * 1e12 << " ps";
    time_label.setText(qs);

    if ((unsigned)selection_box.currentIndex() < observables.size()) {
        observables[selection_box.currentIndex()]->update(plot, time_index);
    }
}

#endif
