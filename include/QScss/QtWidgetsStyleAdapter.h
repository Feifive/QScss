#pragma once

#include "QScss/StyleManager.h"

#include <QApplication>

namespace QScss::QtWidgets {

StyleManager createStyleManager(QApplication &application);
bool applyStyleSheet(QApplication &application, const std::string &qss);

} // namespace QScss::QtWidgets

