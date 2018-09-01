#-------------------------------------------------
#
# Project created by QtCreator 2015-07-27T01:31:30
#
#-------------------------------------------------

QT	+= core gui
QT	+= network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MoveMouse
TEMPLATE = app

LIBS += -framework ApplicationServices


SOURCES += main.cpp\
	mainwindow.cpp \
	mouse.cpp

HEADERS  += mainwindow.h \
	mouse.h

FORMS    += mainwindow.ui

CONFIG += c++11

OTHER_FILES +=
