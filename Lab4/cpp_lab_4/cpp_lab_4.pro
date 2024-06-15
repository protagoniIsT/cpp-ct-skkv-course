QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++23

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
    Cell.cpp \
    Field.cpp \
    GameOverWindow.cpp \
    MainWindow.cpp \
    SettingsWindow.cpp \
    WinWindow.cpp \
    main.cpp

HEADERS += \
    Cell.h \
    Field.h \
    GameOverWindow.h \
    MainWindow.h \
    SettingsWindow.h \
    WinWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    translations/language_de.qm \
    translations/language_de.ts \
    translations/language_en.qm \
    translations/language_en.ts \
    translations/language_es.qm \
    translations/language_es.ts

RESOURCES += \
    resources.qrc

TRANSLATIONS += translations/language_en.ts \
                translations/language_es.ts \
                translations/language_de.ts \
