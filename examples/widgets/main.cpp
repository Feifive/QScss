#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "QScss/QtWidgetsStyleAdapter.h"

#include <string>
#include <vector>

namespace {

bool useResourceMode(const QStringList &arguments)
{
    return arguments.contains(QStringLiteral("--resource"));
}

std::string toStdString(const QString &value)
{
    return value.toStdString();
}

bool applyResource(QScss::StyleManager &styles, const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Failed to open QSS resource: %s", qPrintable(resourcePath));
        return false;
    }

    std::string error;
    const std::string content = file.readAll().toStdString();
    if (!styles.applyContent(content, &error)) {
        qWarning("%s", error.c_str());
        return false;
    }

    return true;
}

bool applyTheme(QScss::StyleManager &styles, const QString &themeName, bool resourceMode)
{
    if (resourceMode) {
        return applyResource(styles, QStringLiteral(":/qscss/%1.qss").arg(themeName));
    }

    std::string error;
    const bool ok = styles.applyTheme(toStdString(themeName), &error);

    if (!ok) {
        qWarning("%s", error.c_str());
    }
    return ok;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QScss::StyleManager styles = QScss::QtWidgets::createStyleManager(app);
    styles.setThemeDirectory(QSCSS_DIST_DIR);

    const bool resourceMode = useResourceMode(app.arguments());
    applyTheme(styles, QStringLiteral("light"), resourceMode);

    QWidget window;
    window.setWindowTitle(QStringLiteral("QScss Widgets Example"));

    auto *layout = new QVBoxLayout(&window);

    auto *panel = new QFrame(&window);
    panel->setObjectName(QStringLiteral("Panel"));

    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->addWidget(new QLabel(QStringLiteral("QScss Widgets Example"), panel));

    auto *description = new QLabel(
        QStringLiteral("SCSS is compiled during development/build time. Qt only loads QSS at runtime."),
        panel
    );
    description->setProperty("secondary", true);
    panelLayout->addWidget(description);

    auto *themeSelector = new QComboBox(panel);
    const std::vector<QScss::ThemeInfo> themes = styles.availableThemes();
    if (themes.empty()) {
        themeSelector->addItem(QStringLiteral("Light"), QStringLiteral("light"));
        themeSelector->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));
    } else {
        for (const QScss::ThemeInfo &theme : themes) {
            themeSelector->addItem(
                QString::fromStdString(theme.displayName),
                QString::fromStdString(theme.id)
            );
        }
    }
    panelLayout->addWidget(themeSelector);

    auto *input = new QLineEdit(panel);
    input->setPlaceholderText(QStringLiteral("QLineEdit styled by compiled QSS"));
    panelLayout->addWidget(input);

    auto *primaryButton = new QPushButton(QStringLiteral("Primary action"), panel);
    primaryButton->setProperty("primary", true);
    panelLayout->addWidget(primaryButton);

    auto *secondaryButton = new QPushButton(QStringLiteral("Secondary action"), panel);
    panelLayout->addWidget(secondaryButton);

    layout->addWidget(panel);

    QObject::connect(themeSelector, &QComboBox::currentIndexChanged, &window, [&](int index) {
        applyTheme(styles, themeSelector->itemData(index).toString(), resourceMode);
    });

    window.resize(420, 260);
    window.show();

    return app.exec();
}
