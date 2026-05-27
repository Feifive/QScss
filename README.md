# QScss

QScss 是一个基于 SCSS 预处理的 Qt QSS 工程化工具库。

它不扩展 Qt 样式能力边界，不实现自定义 Sass 编译器，也不在运行时依赖 Node/npm。开发期或构建期使用 Dart Sass 将 SCSS 编译为标准 QSS，Qt 程序运行时只加载编译后的 `.qss`。

## 第一版范围

- SCSS 到 QSS 构建脚本
- 多主题入口编译
- 纯 C++ `QScssCore` 运行时 `StyleManager` 加载 QSS
- `QScssQtWidgets` 适配 Qt Widgets 的 `QApplication::setStyleSheet()`
- CMake custom target 集成
- Qt Resource 与文件路径两种加载方式
- 最小 Qt Widgets 示例

## 分层

```text
QScssCore
  纯 C++，不依赖 Qt，用于读取 QSS、管理主题状态、调用样式应用回调

QScssQtWidgets
  依赖 Qt Widgets，提供 QApplication 适配器
```

## 目录约定

```text
styles/
  tokens/       设计变量
  mixins/       QSS 复用片段
  components/   组件样式模块
  themes/       主题入口，编译为 dist/qss/*.qss
dist/qss/        编译产物
```

## 构建 QSS

```bash
npm install
npm run build:qss
```

构建脚本会扫描 `styles/themes/*.scss`，并输出同名 `.qss` 到 `dist/qss/`。
同时会生成 `dist/qss/themes.json` 主题清单：

```json
[
  {
    "id": "dark",
    "name": "Dark",
    "file": "dark.qss"
  }
]
```

## CMake 集成

项目默认添加 `qscss_build_styles` 目标：

```bash
cmake -S . -B build
cmake --build build --target qscss_build_styles
```

也可以在外部项目中包含 `cmake/QScssStyles.cmake` 后使用：

```cmake
qscss_add_style_target(my_styles
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
```

## Qt 运行时加载

文件路径加载：

```cpp
QScss::StyleManager styles = QScss::QtWidgets::createStyleManager(app);
styles.setThemeDirectory("/path/to/dist/qss");
styles.applyTheme("light");
styles.currentTheme(); // "light"

for (const QScss::ThemeInfo &theme : styles.availableThemes()) {
    // theme.id / theme.displayName / theme.filePath
}
```

Qt Resource 加载由 Qt 侧读取资源内容，再交给纯 C++ `StyleManager` 应用：

```cpp
QFile file(":/qscss/light.qss");
file.open(QIODevice::ReadOnly | QIODevice::Text);
styles.applyContent(file.readAll().toStdString());
```

`StyleManager` 也预留了热更新开关接口：

```cpp
styles.setHotReloadEnabled(true);
styles.isHotReloadEnabled();
```

## 示例

```bash
cmake -S . -B build
cmake --build build --target qscss_widgets_example
./build/examples/widgets/qscss_widgets_example
./build/examples/widgets/qscss_widgets_example --resource
```
