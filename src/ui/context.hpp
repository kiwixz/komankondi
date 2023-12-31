#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

#include "game/game.hpp"

namespace komankondi::ui {

struct Context : QObject {
public:
    Q_INVOKABLE QString description() const;
    Q_INVOKABLE bool submit(const QString& word);

private:
    game::Game game_;

    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
};

}  // namespace komankondi::ui
