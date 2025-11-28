import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.6

Page {
    id: mainPage
    objectName: "mainPage"
    allowedOrientations: Orientation.All

    // Подключение к C++ backend
    Connections {
        target: appleDetector
        onAnalysisComplete: {
            resultText.text = "Результат: " + result
            confidenceText.text = "Уверенность: " + (confidence * 100).toFixed(1) + "%"

            // Определяем цвет в зависимости от результата
            if (result === "хорошее") {
                resultText.color = "#4CAF50" // Зеленый
            } else if (result === "плохое") {
                resultText.color = "#F44336" // Красный
            } else {
                resultText.color = "#FF9800" // Оранжевый
            }

            resultPanel.visible = true
        }

        onErrorOccurred: {
            errorLabel.text = error
            errorLabel.visible = true
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: "О программе"
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: "Обучить модель"
                onClicked: pageStack.push(Qt.resolvedUrl("TrainingPage.qml"))
            }
            MenuItem {
                text: "📷 Камера"
                enabled: appleDetector.cameraHandler.hasCamera
                onClicked: pageStack.push(Qt.resolvedUrl("CameraPage.qml"))
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "AurСад - Детектор свежести яблок"
            }

            // Область предпросмотра изображения
            Item {
                width: parent.width
                height: width * 0.75

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: Theme.paddingLarge
                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
                    border.color: Theme.highlightColor
                    border.width: 2
                    radius: 10

                    Image {
                        id: previewImage
                        anchors.fill: parent
                        anchors.margins: 5
                        fillMode: Image.PreserveAspectFit
                        source: ""

                        Label {
                            anchors.centerIn: parent
                            text: "Загрузите фото яблока"
                            color: Theme.secondaryColor
                            visible: previewImage.source == ""
                        }
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: appleDetector.isProcessing
                        size: BusyIndicatorSize.Large
                    }
                }
            }

            // Панель с результатом
            Rectangle {
                id: resultPanel
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                height: resultColumn.height + Theme.paddingLarge * 2
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.2)
                radius: 10
                visible: false

                Column {
                    id: resultColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge * 2
                    spacing: Theme.paddingSmall

                    Label {
                        id: resultText
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: Theme.fontSizeExtraLarge
                        font.bold: true
                        text: ""
                    }

                    Label {
                        id: confidenceText
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pixelSize: Theme.fontSizeLarge
                        color: Theme.secondaryColor
                        text: ""
                    }
                }
            }

            // Сообщение об ошибке
            Label {
                id: errorLabel
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.errorColor
                visible: false
                text: ""
            }

            // Кнопки управления
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingLarge

                Button {
                    text: "Выбрать фото"
                    enabled: !appleDetector.isProcessing
                    onClicked: {
                        filePickerDialog.open()
                    }
                }

                Button {
                    text: "Анализировать"
                    enabled: !appleDetector.isProcessing &&
                             previewImage.source != "" &&
                             appleDetector.modelTrained
                    onClicked: {
                        resultPanel.visible = false
                        errorLabel.visible = false
                        appleDetector.analyzeImage(mainPage.selectedImagePath)
                    }
                }
            }

            // Информация о статусе модели
            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: appleDetector.modelTrained ? Theme.highlightColor : Theme.secondaryHighlightColor
                text: appleDetector.modelTrained ?
                      "✓ Модель обучена и готова к работе" :
                      "⚠ Модель не обучена. Перейдите в меню для обучения"
                font.pixelSize: Theme.fontSizeSmall
            }

            // Описание приложения
            SectionHeader {
                text: "Как использовать"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "1. Обучите модель (через меню)\n" +
                      "2. Выберите фото яблока\n" +
                      "3. Нажмите \"Анализировать\"\n" +
                      "4. Получите результат о свежести яблока"
            }

            Item { height: Theme.paddingLarge }
        }
    }

    // Диалог выбора файла (упрощенная версия)
    property string selectedImagePath: ""

    Dialog {
        id: filePickerDialog
        allowedOrientations: Orientation.All

        SilicaFlickable {
            anchors.fill: parent
            contentHeight: fileColumn.height

            Column {
                id: fileColumn
                width: parent.width
                spacing: Theme.paddingMedium

                DialogHeader {
                    title: "Выберите изображение"
                }

                Label {
                    width: parent.width - Theme.paddingLarge * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    wrapMode: Text.WordWrap
                    text: "Выберите изображение из тестового датасета:"
                    color: Theme.highlightColor
                }

                Repeater {
                    model: ListModel {
                        id: imageListModel
                        Component.onCompleted: {
                            // Добавляем примеры из датасета
                            for (var i = 1; i <= 71; i++) {
                                append({
                                    name: i + ".jpg",
                                    path: "/usr/share/ru.auroraos.aurcad/dataset/omsk/Training/" + i + ".jpg"
                                })
                            }
                        }
                    }

                    delegate: BackgroundItem {
                        width: parent.width
                        height: Theme.itemSizeMedium

                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: Theme.paddingLarge
                            text: model.name
                        }

                        onClicked: {
                            mainPage.selectedImagePath = model.path
                            previewImage.source = "file://" + model.path
                            resultPanel.visible = false
                            errorLabel.visible = false
                            filePickerDialog.accept()
                        }
                    }
                }
            }
        }
    }
}
