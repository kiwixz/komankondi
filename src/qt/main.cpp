#include <QGuiApplication>
#include <QQmlError>
#include <QQuickView>

int main(int argc, char** argv) {
    QGuiApplication app{argc, argv};
    QQuickView view{{"qrc:ui/main.qml"}};
    if (!view.errors().empty())
        return 1;
    view.show();
    return app.exec();
}