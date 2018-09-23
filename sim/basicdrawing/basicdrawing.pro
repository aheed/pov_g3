QT += widgets

HEADERS       = renderarea.h \
                window.h \
    povgeometry_g3.h \
    ldserver.h \
    ldprotocol.h

SOURCES       = main.cpp \
                renderarea.cpp \
                window.cpp \
    bmp.cpp \
    povgeometry_gen3.c \
    ldserver.c

RESOURCES     =

INCLUDEPATH += $$PWD/../..

