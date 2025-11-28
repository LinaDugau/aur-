import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: trainingPage
    allowedOrientations: Orientation.All

    property bool isTraining: false
    property real trainingProgress: 0.0
    property string trainingStatus: ""

    Connections {
        target: appleDetector

        onTrainingProgress: {
            trainingPage.trainingProgress = progress / 100.0
            trainingPage.trainingStatus = message
        }

        onTrainingComplete: {
            trainingPage.isTraining = false
            if (success) {
                resultLabel.text = "✓ Обучение успешно завершено!\n" +
                                  "Точность модели: " + (accuracy * 100).toFixed(1) + "%"
                resultLabel.color = Theme.highlightColor
            } else {
                resultLabel.text = "✗ Ошибка при обучении модели"
                resultLabel.color = Theme.errorColor
            }
        }

        onErrorOccurred: {
            trainingPage.isTraining = false
            resultLabel.text = "Ошибка: " + error
            resultLabel.color = Theme.errorColor
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Обучение модели"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                text: "Обучение модели машинного обучения для определения свежести яблок"
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeMedium
            }

            SectionHeader {
                text: "Информация о датасете"
            }

            DetailItem {
                label: "Путь к датасету"
                value: "/usr/share/ru.auroraos.aurcad/dataset/omsk/Training"
            }

            DetailItem {
                label: "Количество изображений"
                value: "57 изображений"
            }

            DetailItem {
                label: "Классы"
                value: "Хорошее / Плохое"
            }

            SectionHeader {
                text: "Параметры обучения"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "• Алгоритм: Random Forest / Logistic Regression\n" +
                      "• Признаки: Цветовые гистограммы, текстурные признаки\n" +
                      "• Валидация: 80% обучение, 20% тест\n" +
                      "• Библиотека: MLPack"
            }

            Item { height: Theme.paddingLarge }

            // Прогресс обучения
            Rectangle {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                height: progressColumn.height + Theme.paddingLarge * 2
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
                radius: 10
                visible: trainingPage.isTraining

                Column {
                    id: progressColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge * 2
                    spacing: Theme.paddingMedium

                    BusyIndicator {
                        anchors.horizontalCenter: parent.horizontalCenter
                        running: trainingPage.isTraining
                        size: BusyIndicatorSize.Large
                    }

                    ProgressBar {
                        width: parent.width
                        minimumValue: 0.0
                        maximumValue: 1.0
                        value: trainingPage.trainingProgress
                    }

                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: trainingPage.trainingStatus
                        color: Theme.highlightColor
                    }

                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: (trainingPage.trainingProgress * 100).toFixed(0) + "%"
                        font.pixelSize: Theme.fontSizeHuge
                        color: Theme.primaryColor
                    }
                }
            }

            // Результат обучения
            Label {
                id: resultLabel
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeMedium
                text: ""
                visible: text != ""
            }

            Item { height: Theme.paddingLarge }

            // Кнопка запуска обучения
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: trainingPage.isTraining ? "Обучение..." : "Начать обучение"
                enabled: !trainingPage.isTraining
                preferredWidth: Theme.buttonWidthLarge

                onClicked: {
                    trainingPage.isTraining = true
                    trainingPage.trainingProgress = 0.0
                    resultLabel.text = ""
                    appleDetector.trainModel("/usr/share/ru.auroraos.aurcad/dataset/omsk/Training", false)
                }
            }

            SectionHeader {
                text: "Дополнительно"
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    text: "✓ Автосохранение включено"
                    enabled: false
                }

                Button {
                    text: "✓ Автозагрузка включена"
                    enabled: false
                }
            }

            Item { height: Theme.paddingLarge }
        }
    }
}
