QT       += core gui concurrent sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



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
        iSpecIDApp/dbconnection.cpp \
        iSpecIDApp/mainwindow.cpp \
        iSpecIDApp/projectselectiondialog.cpp \
        iSpecIDApp/recordmodel.cpp \
        iSpecIDApp/resultsmodel.cpp \
        iSpecIDApp/node.cpp \
        iSpecIDApp/edge.cpp \
        iSpecIDApp/graphscene.cpp \
        iSpecIDApp/gradingoptionsdialog.cpp \
        iSpecIDApp/filterdialog.cpp \
        iSpecIDCore/src/annotator.cpp \
        iSpecIDCore/src/iengine.cpp \
        iSpecIDCore/src/network.cpp


HEADERS += \
        iSpecIDApp/dbconnection.h \
        iSpecIDApp/projectselectiondialog.h \
        iSpecIDApp/recordmodel.h \
        iSpecIDApp/resultsmodel.h \
        iSpecIDApp/qrecord.h \
        iSpecIDApp/node.h \
        iSpecIDApp/edge.h \
        iSpecIDApp/graphscene.h \
        iSpecIDApp/filterItem.h \
        iSpecIDApp/gradingoptionsdialog.h \
        iSpecIDApp/filterdialog.h \
        iSpecIDApp/filterOp.h \
        iSpecIDApp/filterscrollarea.h \
        iSpecIDCore/include/iengine.h \
        iSpecIDCore/include/annotator.h \
        iSpecIDCore/include/datatypes.h \
        iSpecIDCore/include/fileio.h \
        iSpecIDCore/include/network.h \
        iSpecIDCore/include/ispecid.h \
        iSpecIDCore/include/utils.h \
        iSpecIDCore/include/csv.hpp \
        iSpecIDApp/mainwindow.h


FORMS += \
    iSpecIDApp/mainwindow.ui \
    iSpecIDApp/projectselectiondialog.ui \
    iSpecIDApp/gradingoptionsdialog.ui \
    iSpecIDApp/filterdialog.ui

RESOURCES += icons.qrc

win32:{
        CONFIG += c++17
        INCLUDEPATH += "iSpecIDCore/include"
        INCLUDEPATH += "C:\boost\include\boost-1_72"
        INCLUDEPATH += $$PWD/../../../../../boost/include
        DEPENDPATH += $$PWD/../../../../../boost/include
        LIBS += -L$$PWD/../../../../../boost/lib/ -llibboost_regex-mgw81-mt-x64-1_72.dll
        LIBS += -L$$PWD/../../../../../boost/lib/ -llibboost_thread-mgw81-mt-x64-1_72.dll

        LIBS += -L$$PWD/winbuild/curl-7.70.0-win64-mingw/lib/ -llibcurl.dll
        INCLUDEPATH += $$PWD/winbuild/curl-7.70.0-win64-mingw/include
        DEPENDPATH += $$PWD/winbuild/curl-7.70.0-win64-mingw/include
        PRE_TARGETDEPS += $$PWD/winbuild/curl-7.70.0-win64-mingw/lib/libcurl.dll.a
        RC_ICONS = icons/main_icon.ico
}
macx:{
        CONFIG += c++17 app_bundle
        INCLUDEPATH += "iSpecIDCore/include"
        #Boost
        INCLUDEPATH += "/usr/local/Cellar/boost/1.72.0_2/include/"
        LIBS += -L/usr/local/Cellar/boost/1.72.0_2/lib/ -lboost_regex -lboost_thread-mt
        #Curl
        LIBS += -L/usr/local/opt/curl/lib/ -lcurl.4
        INCLUDEPATH += "/usr/local/opt/curl/include"
        DEPENDPATH += "/usr/local/opt/curl/include"
        #ICON
        ICON = icons/main_icon.icns
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
