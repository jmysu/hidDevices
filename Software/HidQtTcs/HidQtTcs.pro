QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11
CONFIG += sdk_no_version_check

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ColorWheel/ColorWheel.cpp \
    Swatches/swatches.cpp \
    kled.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_CIE.cpp \
    mainwindow_Checker.cpp \
    mainwindow_HID.cpp \
    mainwindow_LED.cpp \
    mainwindow_TCS.cpp \
    qcustomplot.cpp \
    tempcolor/source/tempcolor.cpp

HEADERS += \
    ColorWheel/ColorWheel.h \
    Swatches/swatches.h \
    hid_usagePages.h \
    kled.h \
    mainwindow.h \
    qcustomplot.h \
    tempcolor/source/tempcolor.h

FORMS += \
    mainwindow.ui

CONFIG(release, debug|release):{
    message(Release build!)#will be displayed
    DESTDIR = build/release
    message($$DESTDIR)
    macx: LIBS += -L$$PWD/HIDAPI/mac/ -lHIDAPI_Debug
    }

CONFIG(debug, debug|release):{
    message(Debug build!) #not displayed
    DESTDIR = build/debug
    message($$DESTDIR)
    macx: LIBS += -L$$PWD/HIDAPI/mac/ -lHIDAPI_Debug
    }


OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.u

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32: LIBS += -L$$PWD/HIDAPI/windows/ -lHIDAPI -lSetupAPI
INCLUDEPATH += $$PWD/ColorWheel
INCLUDEPATH += $$PWD/Swatches


#DEPENDPATH += $$PWD/HIDAPI
#macx: PRE_TARGETDEPS += $$PWD/HIDAPI/mac/libHIDAPI.a

macx: ICON = USB.icns

RESOURCES += \
    resources.qrc

DISTFILES += \
    CCMatrix.txt \
    CIE-1931.svg \
    MacBethCC.txt \
    blink1.txt
