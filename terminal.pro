QT += widgets serialport
requires(qtConfig(combobox))

TARGET = terminal
TEMPLATE = app

CONFIG		+= debug

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    motion_uart.cpp \
    crc.cpp \
    hexdump.cpp \
    aes.cpp \
    sha256.cpp 

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    motion_uart.hpp \
    config_widget.h \
    aes.h \
    sha256.h 

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
INSTALLS += target
