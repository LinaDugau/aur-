import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: aboutPage
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "О приложении"
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/icons/172x172/aurcad.png"
                width: Theme.iconSizeExtraLarge
                height: Theme.iconSizeExtraLarge
                smooth: true
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "AurСад"
                font.pixelSize: Theme.fontSizeExtraLarge
                font.bold: true
                color: Theme.highlightColor
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "ФруктAIметр"
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryHighlightColor
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Версия 1.0.0"
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }

            Item { height: Theme.paddingLarge }

            SectionHeader {
                text: "Описание"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.primaryColor
                text: "Приложение для определения свежести яблок с использованием " +
                      "искусственного интеллекта и компьютерного зрения."
            }

            SectionHeader {
                text: "Технологии"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "• C++ и Qt 5\n" +
                      "• QML (Sailfish Silica)\n" +
                      "• OpenCV - обработка изображений\n" +
                      "• MLPack - машинное обучение\n" +
                      "• ОС Аврора\n" +
                      "• Conan - управление зависимостями"
            }

            SectionHeader {
                text: "Возможности"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "✓ Определение свежести яблок по фотографии\n" +
                      "✓ Локальное обучение ML-модели\n" +
                      "✓ Работа без интернета\n" +
                      "✓ Классификация: хорошее/плохое/не яблоко\n" +
                      "✓ Показатель уверенности модели"
            }

            SectionHeader {
                text: "Алгоритм работы"
            }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: "1. Получение изображения с камеры или из файла\n" +
                      "2. Предобработка изображения (нормализация, resize)\n" +
                      "3. Извлечение признаков (цветовые гистограммы, текстура)\n" +
                      "4. Классификация с помощью MLPack\n" +
                      "5. Вывод результата с уверенностью модели"
            }

            SectionHeader {
                text: "Разработано для"
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Хакатон AuroraOS AI 2025"
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.highlightColor
                font.bold: true
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Омск, 2025"
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }

            Item { height: Theme.paddingLarge }

            Label {
                width: parent.width - Theme.paddingLarge * 2
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "Демонстрация возможностей ОС Аврора в области\n" +
                      "искусственного интеллекта и машинного обучения"
            }

            Item { height: Theme.paddingLarge * 2 }
        }
    }
}
