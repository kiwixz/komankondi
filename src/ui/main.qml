import QtQml
import QtQuick
import QtQuick.Layouts
import komankondi

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
                if (Context.submit(text)) {
                    text = "";
                    description.text = Context.description();
                }
            }
        }

        Text {
            id: description

            Layout.fillWidth: true
            Layout.fillHeight: true

            text: Context.description()
            wrapMode: Text.Wrap
            color: "white"
            font.pointSize: 16
        }
    }
}
