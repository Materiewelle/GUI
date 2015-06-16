QT += core gui widgets printsupport

TARGET = GUI
TEMPLATE = app

SOURCES += main.cpp \
           qcustomplot.cpp

HEADERS += mainwindow.hpp \
           qcustomplot.hpp

FORMS += mainwindow.ui

#LIBS += -lqcustomplot

QMAKE_CXXFLAGS = -std=c++14 -march=native
QMAKE_CXXFLAGS_RELEASE = -O3

