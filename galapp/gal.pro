#-------------------------------------------------
#
# Project created by QtCreator 2018-12-08T16:25:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets qml network

TARGET = gal
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += thirdparty/everything
INCLUDEPATH += src

CONFIG += c++11

SOURCES += \
    src/data.cpp \
    src/main.cpp \
    src/mainbox.cpp \
    src/maintray.cpp \
    src/plugin.cpp \
    src/pluginmanager.cpp \
    src/pydata.cpp \
    src/resultlist.cpp

HEADERS += \
    src/mainbox.h \
    src/data.h \
    src/plugin.h \
    src/pluginmanager.h \
    src/pydata.h \
    src/resultlist.h \
    src/maintray.h \
    include/galcppfreeinterface.h \
    resource.h \
    thirdparty/everything/Everything.h

RC_FILE = gal.rc

TRANSLATIONS = ts/galapp_zh_CN.ts

CONFIG(debug, debug|release) {
    MOC_DIR += $$OUT_PWD/temp/debug/moc
    OBJECTS_DIR += $$OUT_PWD/temp/debug/obj
    #QM_FILES_RESOURCE_PREFIX=$$OUT_PWD/debug/i18n/
}
else {
    MOC_DIR += $$OUT_PWD/temp/release/moc
    OBJECTS_DIR += $$OUT_PWD/temp/release/obj
    #QM_FILES_RESOURCE_PREFIX=$$OUT_PWD/release/i18n/
}

#CONFIG+=lrelease embed_translations   QTBUG-74004

LIBS += User32.lib 
LIBS += "$$PWD/thirdparty/everything/Everything64.lib"

RESOURCES += \
    res/galres.qrc
