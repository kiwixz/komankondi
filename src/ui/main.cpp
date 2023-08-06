#include <chrono>
#include <filesystem>
#include <stdexcept>

#include <QDir>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QtGlobal>
#include <QTimer>
#include <range/v3/algorithm/none_of.hpp>

#include "ui/context.hpp"
#include "utils/log.hpp"

int main(int argc, char** argv) {
    using namespace komankondi;
    using namespace ui;

    try {
        Q_INIT_RESOURCE(main);

        qInstallMessageHandler([](QtMsgType level_qt, const QMessageLogContext& /*from*/, const QString& message) {
            log::Level level = [&] {
                switch (level_qt) {
                case QtDebugMsg: return log::Level::debug;
                case QtInfoMsg: return log::Level::info;
                case QtWarningMsg: return log::Level::warn;
                case QtCriticalMsg:
                case QtFatalMsg: return log::Level::error;
                }
                assert(false);
                return log::Level::debug;
            }();
            log::log(level, "qt: {}", message.toStdString());
        });

        const char* hot_reload = std::getenv("KOMANKONDI_HOT_RELOAD");

        QGuiApplication app{argc, argv};

        QUrl main_qml = QString::fromStdString(fmt::format("{}/main.qml", hot_reload ? hot_reload : "qrc:"));
        QQuickView window{main_qml};
        if (window.status() != QQuickView::Ready)
            return 1;

        QTimer timer;
        if (hot_reload) {
            log::info("enabling hot reload");
            timer.callOnTimeout([&,
                                 hot_reload = std::filesystem::path{hot_reload},
                                 last_reload = std::chrono::file_clock::now()]() mutable noexcept {
                try {
                    std::chrono::file_clock::time_point now = std::chrono::file_clock::now();
                    if (ranges::none_of(std::filesystem::recursive_directory_iterator{hot_reload},
                                        [&](const std::filesystem::directory_entry& entry) { return entry.last_write_time() > last_reload; }))
                    {
                        return;
                    }

                    log::info("reloading qml");
                    window.setSource({});
                    window.engine()->clearComponentCache();
                    window.setSource(main_qml);
                    last_reload = now;
                }
                catch (const std::runtime_error& ex) {
                    log::error("{}", ex.what());
                    app.exit(1);
                }
            });
            timer.setInterval(200);
            timer.start();
        }

        Context ctx;
        window.rootContext()->setContextProperty("ctx", &ctx);

        window.setResizeMode(QQuickView::SizeRootObjectToView);
        window.resize(800, 450);
        window.show();
        return app.exec();
    }
    catch (const std::runtime_error& ex) {
        log::error("{}", ex.what());
        return 1;
    }
}
