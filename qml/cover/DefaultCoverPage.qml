import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    Column {
        anchors.centerIn: parent
        spacing: Theme.paddingMedium

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/icons/128x128/aurcad.png"
            width: Theme.iconSizeLarge
            height: Theme.iconSizeLarge
            smooth: true
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "AurСад"
            font.pixelSize: Theme.fontSizeLarge
            font.bold: true
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: appleDetector.modelTrained ? "Готов" : "Не обучен"
            font.pixelSize: Theme.fontSizeSmall
            color: appleDetector.modelTrained ? Theme.highlightColor : Theme.secondaryColor
        }
    }

    CoverActionList {
        enabled: appleDetector.modelTrained

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
        }
    }
}
