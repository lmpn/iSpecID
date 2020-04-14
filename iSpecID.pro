QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32 {
        CONFIG += c++11
}
macx {
        CONFIG += c++17 app_bundle
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        app.cpp \
        iSpecIDApp/edge.cpp \
        iSpecIDApp/graphscene.cpp \
        iSpecIDApp/node.cpp \
        iSpecIDApp/resultsmodel.cpp \
        iSpecIDApp/mainwindow.cpp \
        iSpecIDApp/recordmodel.cpp \
        iSpecIDCore/src/annotator.cpp \
        iSpecIDCore/src/record.cpp \
        iSpecIDCore/src/iengine.cpp \
        iSpecIDCore/src/miner.cpp \
        iSpecIDCore/src/utils.cpp \
        iSpecIDApp/filterdialog.cpp


HEADERS += \
        iSpecIDCore/include/iengine.h \
        iSpecIDCore/include/annotator.h \
        iSpecIDCore/include/species.h \
        iSpecIDCore/include/miner.h \
        iSpecIDCore/include/record.h \
        iSpecIDCore/include/utils.h \
        iSpecIDCore/include/csv.hpp \
        iSpecIDApp/edge.h \
        iSpecIDApp/graphscene.h \
        iSpecIDApp/node.h \
        iSpecIDApp/resultsmodel.h \
        iSpecIDApp/mainwindow.h \
        iSpecIDApp/recordmodel.h \
        iSpecIDApp/filterdialog.h \
        iSpecIDApp/filterOp.h \
        iSpecIDApp/filterscrollarea.h

FORMS += \
    iSpecIDApp/mainwindow.ui \
    iSpecIDApp/filterdialog.ui

INCLUDEPATH += "iSpecIDCore/include"
INCLUDEPATH += "/usr/local/Cellar/boost/1.72.0/include/"
INCLUDEPATH += "/usr/local/include"

LIBS += "-lcurl"





# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

ICON = icons/main_icon.icns
