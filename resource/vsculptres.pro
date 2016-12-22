TEMPLATE = app

QT += qml quick
CONFIG += c++11

SOURCES +=

RESOURCES += \
    K3DStudio-2.qrc \
    K3DStudio-3.qrc \
    qml.qrc \
    K3DStudio-4.qrc \
    K3DStudio-5.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
