import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "black"

    ColumnLayout {
        anchors.fill: parent

        TextInput {
            Layout.alignment: Qt.AlignHCenter

            focus: true
            color: "white"
            font.pointSize: 48

            onTextChanged: {
                if (c.submit(text)) {
                    text = ""
                    description.text = c.description()
                }
            }
        }

        Text {
            id: description

            Layout.fillWidth: true
            Layout.fillHeight: true

            text: c.description()
            wrapMode: Text.Wrap
            color: "white"
            font.pointSize: 16
        }
    }
}
