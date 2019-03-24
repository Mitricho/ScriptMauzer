TEMPLATE = app
TARGET = ScriptMauzer
QT += script xmlpatterns core xml
//QT -= gui
CONFIG   += qt console

SOURCES += main.cpp

HEADERS += Classes.h \
           QMyDomDocument.h
win32{
    CONFIG(debug, debug|release) {
        DESTDIR = debug/win
        LIBS += -lole32 -loleaut32 -luuid -lStrmiids -lQuartz -lVersion
    } else {
       DESTDIR = release/win
       LIBS += -lole32 -loleaut32 -luuid -lStrmiids -lQuartz -lVersion
    }    
}



