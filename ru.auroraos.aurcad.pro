TARGET = ru.auroraos.aurcad

CONFIG += \
    auroraapp

# Включаем файлы, сгенерированные Conan
!exists($$PWD/conanbuildinfo.pri): warning("Conan dependencies not installed. Run: conan install . --build=missing")
exists($$PWD/conanbuildinfo.pri): include($$PWD/conanbuildinfo.pri)

PKGCONFIG += \

QT += core multimedia gui

SOURCES += \
    src/main.cpp \
    src/AppleDetector.cpp \
    src/ImageProcessor.cpp \
    src/ImageProcessorExtended.cpp \
    src/AppleClassifier.cpp \
    src/CameraHandler.cpp \
    src/SegmentationData.cpp \
    src/ONNXInference.cpp \
    src/YOLO11Segmentation.cpp \
    src/YOLACTInference.cpp \

HEADERS += \
    src/AppleDetector.h \
    src/ImageProcessor.h \
    src/AppleClassifier.h \
    src/CameraHandler.h \
    src/SegmentationData.h \
    src/ONNXInference.h \
    src/YOLO11Segmentation.h \
    src/YOLACTInference.h \

DISTFILES += \
    rpm/ru.auroraos.aurcad.spec \
    qml/*.qml \
    qml/pages/*.qml \
    qml/cover/*.qml \

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += auroraapp_i18n

TRANSLATIONS += \
    translations/ru.auroraos.aurcad.ts \
    translations/ru.auroraos.aurcad-ru.ts \

# Копируем датасет в ресурсы приложения
RESOURCES += \
    resources.qrc

# C++11 поддержка
CONFIG += c++11

# Оптимизация
QMAKE_CXXFLAGS += -O2

# Установка датасета
dataset.files = omsk/*
dataset.path = /usr/share/$${TARGET}/dataset/omsk
INSTALLS += dataset
