######################################################################
# Automatically generated by qmake (3.0) ?? ????. 2 17:55:11 2017
######################################################################

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = yourdroid-qt
INCLUDEPATH += .

# Input
HEADERS += data.h log.h mainclass.h window.h \
    enum.h \
    os.h \
    install.h
FORMS += window.ui
SOURCES += data.cpp \
           log.cpp \
           main.cpp \
           mainclass.cpp \
           window.cpp \
    install.cpp

RESOURCES += \
    resource.qrc

LIBS += -lz
