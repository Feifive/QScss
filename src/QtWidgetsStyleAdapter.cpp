#include "QScss/QtWidgetsStyleAdapter.h"

#include <QString>

namespace QScss::QtWidgets {

StyleManager createStyleManager(QApplication &application)
{
    return StyleManager([&application](const std::string &qss) {
        return applyStyleSheet(application, qss);
    });
}

bool applyStyleSheet(QApplication &application, const std::string &qss)
{
    application.setStyleSheet(QString::fromStdString(qss));
    return true;
}

} // namespace QScss::QtWidgets

