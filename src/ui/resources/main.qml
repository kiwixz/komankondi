import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "black"

    ColumnLayout {
        anchors.fill: parent

        TextInput {
            Layout.alignment: Qt.AlignHCenter

            text: "input"
            focus: true
            color: "white"
            font.pointSize: 48
        }

        Text {
            Layout.fillWidth: true
            Layout.fillHeight: true

            text: "the <b>very</b> long description"
            wrapMode: Text.Wrap
            color: "white"
            font.pointSize: 24
        }
    }
}
