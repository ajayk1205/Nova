TEMPLATE = app
TARGET = NovaGUI
QT += core \
    gui
HEADERS += main.h \ 
    novagui.h
SOURCES += main.cpp \
    novagui.cpp
FORMS += novagui.ui
RESOURCES +=
INCLUDEPATH += /home/victim/Code/NovaGit/Nova/NovaLibrary/src
LIBS += /home/victim/Code/NovaGit/Nova/NovaLibrary/Debug/libNovaLibrary.a \
/usr/lib/libboost_serialization.a \
/usr/lib/libann.a