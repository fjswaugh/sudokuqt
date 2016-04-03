#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T17:14:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sudokuqt
TEMPLATE = app


SOURCES += main.cpp\
           mainwindow.cpp \
           sudoku.cpp

HEADERS  += mainwindow.h \
            sudoku.h

DESTDIR=.
OBJECTS_DIR=build
MOC_DIR=build

FORMS    +=

CONFIG += c++11
