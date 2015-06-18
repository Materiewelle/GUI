QT += core gui widgets printsupport

INCLUDEPATH += .
DEPENDPATH += .

TARGET = GUI
TEMPLATE = app

SOURCES += main.cpp \
    qcustomplot.cpp

HEADERS += \
    qcustomplot.hpp \
    constant.hpp \
    device.hpp \
    observable.hpp \
    graph_data.hpp \
    main_window.hpp

FORMS +=

#LIBS += -lqcustomplot

QMAKE_CXXFLAGS = -std=c++14 -march=native
QMAKE_CXXFLAGS_RELEASE = -O3

