#include "context.hpp"

#include <QString>

namespace komankondi::ui {

QString Context::description() const {
    return QString::fromStdString(game_.description());
}

bool Context::submit(const QString& word) {
    return game_.submit(word.toStdString());
}

}  // namespace komankondi::ui
