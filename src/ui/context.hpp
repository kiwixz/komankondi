#pragma once

#include <QObject>
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
};

}  // namespace komankondi::ui
