QT += \
    core \
    gui

SOURCES += \
    $$PWD/src/QtMessageFilter/qtmessagefilter.cpp

HEADERS += \
    $$PWD/src/QtMessageFilter/qtmessagefilter.h

RESOURCES += \
    $$PWD/share/QtMessageFilter/icons/icons.qrc

INCLUDEPATH += \
    $$PWD/src
