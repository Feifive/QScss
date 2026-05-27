#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace QScss {

struct ThemeInfo
{
    std::string id;
    std::string displayName;
    std::filesystem::path filePath;
};

class StyleManager
{
public:
    using ApplyCallback = std::function<bool(const std::string &)>;

    explicit StyleManager(ApplyCallback applyCallback);

    void setThemeDirectory(std::filesystem::path themeDirectory);
    const std::filesystem::path &themeDirectory() const;

    void setThemeManifestPath(std::filesystem::path manifestPath);
    const std::filesystem::path &themeManifestPath() const;
    std::vector<ThemeInfo> availableThemes() const;

    const std::string &currentTheme() const;
    void setHotReloadEnabled(bool enabled);
    bool isHotReloadEnabled() const;

    bool applyTheme(const std::string &themeName, std::string *errorMessage = nullptr);
    bool applyFile(const std::filesystem::path &qssPath, std::string *errorMessage = nullptr);
    bool applyContent(const std::string &content, std::string *errorMessage = nullptr);

private:
    std::string normalizeThemeName(const std::string &themeName) const;
    std::string normalizeThemeFileName(const std::string &themeName) const;
    std::filesystem::path activeThemeManifestPath() const;
    std::vector<ThemeInfo> loadThemeManifest() const;
    std::filesystem::path resolveThemePath(const std::string &themeName) const;

    ApplyCallback m_applyCallback;
    std::filesystem::path m_themeDirectory;
    std::filesystem::path m_themeManifestPath;
    std::string m_currentTheme;
    bool m_hotReloadEnabled = false;
};

} // namespace QScss
