import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.15

Rectangle {
    width: 800
    height: 450
    color: "green"
    visible: true

    ColumnLayout {
        Text {
            text: "hello world!"
            font.pointSize: 32
            color: "white"
        }
    }
}
