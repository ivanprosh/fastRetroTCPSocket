import QtQuick 2.1
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Dialog {
    id: root;

    //color: "#282C34";
    standardButtons: StandardButton.Close | StandardButton.Reset

    property bool errorFile
    property string tabTitle: MainClass.logger.entries.length > 0 ? "Log <b>" + MainClass.logger.entries.length + "</b>" : "Log"

    ListModel {
        id: errorModel
        ListElement { content: "Ошибка (проверьте существование лог файла и права на доступ)"}
    }
    ListView {
        id: listview
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: parent.top;
        anchors.bottom: footer.top;
        //contentWidth:

        model: errorFile ? errorModel : MainClass.logger.entries;

        delegate: Text {

            anchors.leftMargin: 4;
            anchors.left: parent.left;
            anchors.right: parent.right;
            text: modelData;
            color: enabled ? "#2d2f33" : "#a6b0c1"

            font.family: "Tahoma"
            font.pointSize: 10;

            elide: Text.ElideRight;
            textFormat: Text.PlainText;
            horizontalAlignment : Text.AlignLeft;
        }
    }

    Rectangle {
        id: footer;

        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.bottom: parent.bottom;

        height: footerCol.height;

        color: "#c9ccd3";

        Column {
            id: footerCol;

            anchors.left: parent.left;
            anchors.right: parent.right;
            anchors.bottom: parent.bottom;

            Rectangle {
                anchors.left: parent.left;
                anchors.right: parent.right;
                height: 1;

                color: "#21252B";
            }
            /*
            Button {
                anchors.right: parent.right;
                anchors.rightMargin: 4;
                icon: icons.fa_trash_o;
                label.text: "Clear"

                onClicked: MainClass.logger.clear();
            }
            */
        }
    }
    onReset: {
        MainClass.logger.clear();
        MainClass.setCurrentError();
    }
}
