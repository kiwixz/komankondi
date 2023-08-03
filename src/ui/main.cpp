#include <QDir>
#include <QGuiApplication>
#include <QQmlError>
#include <QQuickView>

int main(int argc, char** argv) {
    Q_INIT_RESOURCE(main);
    QGuiApplication app{argc, argv};
    QQuickView view{{"qrc:main.qml"}};
    if (!view.errors().empty())
        return 1;
    view.show();
    return app.exec();
}
