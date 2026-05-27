#include "QScss/StyleManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <utility>

namespace QScss {
namespace {

std::string toLowerAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool hasQssExtension(const std::string &value)
{
    const std::string lowerValue = toLowerAscii(value);
    return lowerValue.size() >= 4 && lowerValue.substr(lowerValue.size() - 4) == ".qss";
}

std::string readFileContent(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::size_t skipWhitespace(const std::string &text, std::size_t index)
{
    while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index]))) {
        ++index;
    }
    return index;
}

std::optional<std::string> parseJsonString(const std::string &text, std::size_t index)
{
    if (index >= text.size() || text[index] != '"') {
        return std::nullopt;
    }

    std::string value;
    for (++index; index < text.size(); ++index) {
        const char ch = text[index];
        if (ch == '"') {
            return value;
        }
        if (ch == '\\' && index + 1 < text.size()) {
            value.push_back(text[++index]);
            continue;
        }
        value.push_back(ch);
    }
    return std::nullopt;
}

std::optional<std::string> extractStringField(const std::string &object, const std::string &key)
{
    const std::size_t keyIndex = object.find('"' + key + '"');
    if (keyIndex == std::string::npos) {
        return std::nullopt;
    }

    const std::size_t colonIndex = object.find(':', keyIndex);
    if (colonIndex == std::string::npos) {
        return std::nullopt;
    }

    return parseJsonString(object, skipWhitespace(object, colonIndex + 1));
}

} // namespace

StyleManager::StyleManager(ApplyCallback applyCallback)
    : m_applyCallback(std::move(applyCallback))
{
}

void StyleManager::setThemeDirectory(std::filesystem::path themeDirectory)
{
    m_themeDirectory = std::move(themeDirectory);
}

const std::filesystem::path &StyleManager::themeDirectory() const
{
    return m_themeDirectory;
}

void StyleManager::setThemeManifestPath(std::filesystem::path manifestPath)
{
    m_themeManifestPath = std::move(manifestPath);
}

const std::filesystem::path &StyleManager::themeManifestPath() const
{
    return m_themeManifestPath;
}

std::vector<ThemeInfo> StyleManager::availableThemes() const
{
    return loadThemeManifest();
}

const std::string &StyleManager::currentTheme() const
{
    return m_currentTheme;
}

void StyleManager::setHotReloadEnabled(bool enabled)
{
    m_hotReloadEnabled = enabled;
}

bool StyleManager::isHotReloadEnabled() const
{
    return m_hotReloadEnabled;
}

bool StyleManager::applyTheme(const std::string &themeName, std::string *errorMessage)
{
    if (m_themeDirectory.empty()) {
        if (errorMessage) {
            *errorMessage = "Theme directory is not configured.";
        }
        return false;
    }

    const std::filesystem::path themePath = resolveThemePath(themeName);
    if (!applyFile(themePath, errorMessage)) {
        return false;
    }

    m_currentTheme = normalizeThemeName(themeName);
    for (const ThemeInfo &theme : loadThemeManifest()) {
        if (theme.filePath == themePath || toLowerAscii(theme.id) == toLowerAscii(themeName)) {
            m_currentTheme = theme.id;
            break;
        }
    }
    return true;
}

bool StyleManager::applyFile(const std::filesystem::path &qssPath, std::string *errorMessage)
{
    std::error_code errorCode;
    if (!std::filesystem::exists(qssPath, errorCode) || errorCode) {
        if (errorMessage) {
            *errorMessage = "QSS file does not exist: " + qssPath.string();
        }
        return false;
    }

    if (!std::filesystem::is_regular_file(qssPath, errorCode) || errorCode) {
        if (errorMessage) {
            *errorMessage = "QSS path is not a regular file: " + qssPath.string();
        }
        return false;
    }

    std::ifstream file(qssPath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        if (errorMessage) {
            *errorMessage = "Failed to open QSS file: " + qssPath.string();
        }
        return false;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    if (file.bad()) {
        if (errorMessage) {
            *errorMessage = "Failed to read QSS file: " + qssPath.string();
        }
        return false;
    }

    return applyContent(stream.str(), errorMessage);
}

bool StyleManager::applyContent(const std::string &content, std::string *errorMessage)
{
    if (!m_applyCallback) {
        if (errorMessage) {
            *errorMessage = "Style apply callback is not configured.";
        }
        return false;
    }

    if (!m_applyCallback(content)) {
        if (errorMessage) {
            *errorMessage = "Style apply callback returned false.";
        }
        return false;
    }

    return true;
}

std::string StyleManager::normalizeThemeName(const std::string &themeName) const
{
    const std::string fileName = normalizeThemeFileName(themeName);
    return std::filesystem::path(fileName).stem().string();
}

std::string StyleManager::normalizeThemeFileName(const std::string &themeName) const
{
    std::string fileName = toLowerAscii(themeName);
    if (!hasQssExtension(fileName)) {
        fileName += ".qss";
    }
    return fileName;
}

std::filesystem::path StyleManager::activeThemeManifestPath() const
{
    if (!m_themeManifestPath.empty()) {
        return m_themeManifestPath;
    }
    if (m_themeDirectory.empty()) {
        return {};
    }
    return m_themeDirectory / "themes.json";
}

std::vector<ThemeInfo> StyleManager::loadThemeManifest() const
{
    const std::filesystem::path manifestPath = activeThemeManifestPath();
    std::error_code errorCode;
    if (manifestPath.empty() || !std::filesystem::is_regular_file(manifestPath, errorCode) || errorCode) {
        return {};
    }

    const std::string content = readFileContent(manifestPath);
    std::vector<ThemeInfo> themes;
    for (std::size_t start = content.find('{'); start != std::string::npos; start = content.find('{', start + 1)) {
        const std::size_t end = content.find('}', start + 1);
        if (end == std::string::npos) {
            break;
        }

        const std::string object = content.substr(start, end - start + 1);
        const auto id = extractStringField(object, "id");
        const auto file = extractStringField(object, "file");
        if (!id || !file) {
            continue;
        }

        const auto name = extractStringField(object, "name");
        const std::filesystem::path filePath = std::filesystem::path(*file).is_absolute()
            ? std::filesystem::path(*file)
            : manifestPath.parent_path() / *file;
        themes.push_back(ThemeInfo{*id, name.value_or(*id), filePath});
    }
    return themes;
}

std::filesystem::path StyleManager::resolveThemePath(const std::string &themeName) const
{
    const std::string normalizedThemeName = normalizeThemeName(themeName);
    for (const ThemeInfo &theme : loadThemeManifest()) {
        if (toLowerAscii(theme.id) == normalizedThemeName) {
            return theme.filePath;
        }
    }

    return m_themeDirectory / normalizeThemeFileName(themeName);
}

} // namespace QScss
