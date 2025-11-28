import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.6

Page {
    id: cameraPage
    allowedOrientations: Orientation.All

    property bool analyzing: false
    property string detectionResult: ""

    Component.onCompleted: {
        // Запускаем камеру при открытии страницы
        if (appleDetector.cameraHandler.hasCamera) {
            appleDetector.cameraHandler.startCamera()
        }
    }

    Component.onDestruction: {
        // Останавливаем камеру при закрытии
        appleDetector.cameraHandler.stopCamera()
    }

    Connections {
        target: appleDetector

        onAnalysisComplete: {
            analyzing = false
            detectionResult = result
            confidenceLabel.text = "Уверенность: " + (confidence * 100).toFixed(1) + "%"

            // Определяем цвет
            if (result === "хорошее") {
                resultLabel.color = "#4CAF50" // Зеленый
            } else if (result === "плохое") {
                resultLabel.color = "#F44336" // Красный
            } else {
                resultLabel.color = "#FF9800" // Оранжевый
            }
        }

        onApplesDetected: {
            detectionsLabel.text = "Обнаружено яблок: " + count
        }

        onErrorOccurred: {
            analyzing = false
            errorLabel.text = error
            errorLabel.visible = true
        }
    }

    Connections {
        target: appleDetector.cameraHandler

        onImageCaptured: {
            // Автоматически анализируем захваченное изображение
            analyzing = true
            resultLabel.text = ""
            confidenceLabel.text = ""
            appleDetector.analyzeImage(filePath)
        }

        onErrorOccurred: {
            errorLabel.text = error
            errorLabel.visible = true
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Камера - Real-time детекция"
            }

            // Viewfinder для предпросмотра камеры
            Item {
                width: parent.width
                height: width * 0.75

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium
                    color: "black"
                    border.color: Theme.highlightColor
                    border.width: 2
                    radius: 10

                    Camera {
                        id: camera

                        Component.onCompleted: {
                            // Настройка камеры
                            deviceId = appleDetector.cameraHandler.hasCamera ?
                                       QtMultimedia.defaultCamera.deviceId : ""
                        }

                        imageCapture {
                            onImageCaptured: {
                                // Предпросмотр захваченного изображения
                                photoPreview.source = preview
                            }
                        }
                    }

                    VideoOutput {
                        id: videoOutput
                        source: camera
                        anchors.fill: parent
                        anchors.margins: 5
                        fillMode: VideoOutput.PreserveAspectFit
                        autoOrientation: true

                        // Наложение результатов детекции
                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 10
                            height: overlayColumn.height + 20
                            color: Theme.rgba(Theme.highlightBackgroundColor, 0.8)
                            radius: 5
                            visible: realtimeSwitch.checked

                            Column {
                                id: overlayColumn
                                anchors.centerIn: parent
                                spacing: 5

                                Label {
                                    id: detectionsLabel
                                    text: "Поиск яблок..."
                                    color: Theme.primaryColor
                                    font.pixelSize: Theme.fontSizeSmall
                                }
                            }
                        }
                    }

                    Image {
                        id: photoPreview
                        anchors.fill: parent
                        anchors.margins: 5
                        fillMode: Image.PreserveAspectFit
                        visible: source != ""
                        smooth: true
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: analyzing
                        size: BusyIndicatorSize.Large
                    }

                    // Кнопка закрытия превью
                    IconButton {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: Theme.paddingMedium
                        icon.source: "image://theme/icon-m-clear"
                        visible: photoPreview.visible

                        onClicked: {
                            photoPreview.source = ""
                            resultLabel.text = ""
                            confidenceLabel.text = ""
                        }
                    }
                }
            }

            // Результат анализа
            Rectangle {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                height: resultColumn.height + Theme.paddingLarge * 2
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.2)
                radius: 10
                visible: resultLabel.text != ""

                Column {
                    id: resultColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge * 2
                    spacing: Theme.paddingSmall

                    Label {
                        id: resultLabel
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: Theme.fontSizeHuge
                        font.bold: true
                        text: detectionResult
                    }

                    Label {
                        id: confidenceLabel
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: Theme.fontSizeMedium
                        color: Theme.secondaryColor
                        text: ""
                    }
                }
            }

            // Ошибки
            Label {
                id: errorLabel
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.errorColor
                visible: false
            }

            // Управление
            SectionHeader {
                text: "Управление"
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingLarge

                Button {
                    text: "Снимок"
                    enabled: !analyzing && appleDetector.cameraHandler.isActive
                    onClicked: {
                        appleDetector.cameraHandler.captureImage()
                    }
                }

                Button {
                    text: analyzing ? "Анализ..." : "Анализировать"
                    enabled: !analyzing && photoPreview.source != ""
                    visible: photoPreview.source != ""
                    onClicked: {
                        analyzing = true
                        // Повторный анализ текущего снимка
                    }
                }
            }

            // Real-time режим
            TextSwitch {
                id: realtimeSwitch
                text: "Real-time детекция"
                description: "Непрерывный поиск яблок в кадре"
                enabled: appleDetector.modelTrained && appleDetector.cameraHandler.isActive

                onCheckedChanged: {
                    appleDetector.setRealtimeAnalysis(checked)
                }
            }

            // Использование сегментации
            TextSwitch {
                text: "Использовать polygon данные"
                description: "Улучшенное извлечение признаков"
                checked: appleDetector.useSegmentation

                onCheckedChanged: {
                    appleDetector.useSegmentation = checked
                }
            }

            // Информация
            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "Наведите камеру на яблоко и сделайте снимок. " +
                      "В режиме real-time анализ происходит автоматически."
            }

            // Статус камеры
            DetailItem {
                label: "Статус камеры"
                value: appleDetector.cameraHandler.isActive ? "Активна" : "Неактивна"
            }

            DetailItem {
                label: "Модель"
                value: appleDetector.modelTrained ? "Обучена" : "Не обучена"
            }

            Item { height: Theme.paddingLarge }
        }
    }
}
