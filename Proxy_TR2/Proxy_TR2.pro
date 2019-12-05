<<<<<<< HEAD:Proxy_TR2/Proxy_TR2.pro
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

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
    connection.cpp \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    parser.cpp \
    proxyserver.cpp \
    proxywindow.cpp \
    tasks.cpp \
    tools.cpp

HEADERS += \
    connection.h \
    errors.h \
    log.h \
    mainwindow.h \
    parser.h \
    proxyserver.h \
    proxywindow.h \
    tasks.h \
    tools.h

FORMS += \
    log.ui \
    mainwindow.ui \
    proxywindow.ui \
    tasks.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
=======
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# O Define abaixo faz seu compilador avisar se vc esta usando features do Qt depreciadas
DEFINES += QT_DEPRECATED_WARNINGS

# Se descomentar o define abaixo o codigo nao compila quando existem features depreciadas.
# Pode desabilitar features anteriores a uma certa versão do Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # desabilita APIs depreciadas antes do Qt 6.0.0

SOURCES += \
    connection.cpp \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    parser.cpp \
    proxyserver.cpp \
    proxywindow.cpp \
    tools.cpp

HEADERS += \
    connection.h \
    log.h \
    mainwindow.h \
    parser.h \
    proxyserver.h \
    proxywindow.h \
    tools.h \
    errors.h

FORMS += \
    log.ui \
    mainwindow.ui \
    proxywindow.ui

#Regras padrao para desenvolvimento.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
>>>>>>> 48e269759fb5bb2848c9a9864d4f2434b7c6c9a1:Proxy_HTTP/Proxy_HTTP.pro
