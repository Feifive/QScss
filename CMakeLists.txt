cmake_minimum_required(VERSION 3.21)

project(QScss
    VERSION 0.1.0
    DESCRIPTION "Qt QSS engineering toolkit powered by SCSS preprocessing"
    LANGUAGES CXX
)

option(QSCSS_BUILD_EXAMPLES "Build QScss examples" ON)
option(QSCSS_BUILD_QT_WIDGETS_ADAPTER "Build Qt Widgets adapter target" ON)
option(QSCSS_ADD_STYLE_TARGET "Add qscss_build_styles custom target" ON)

add_library(QScssCore
    src/StyleManager.cpp
    include/QScss/StyleManager.h
)

add_library(QScss::Core ALIAS QScssCore)

target_compile_features(QScssCore PUBLIC cxx_std_17)

target_include_directories(QScssCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(cmake/QScssStyles.cmake)

if(QSCSS_ADD_STYLE_TARGET)
    qscss_add_style_target(qscss_build_styles
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

if(QSCSS_BUILD_QT_WIDGETS_ADAPTER OR QSCSS_BUILD_EXAMPLES)
    if(NOT DEFINED QT_SDK_DIR OR QT_SDK_DIR STREQUAL "")
        if(DEFINED ENV{QT_ROOT_DIR} AND NOT "$ENV{QT_ROOT_DIR}" STREQUAL "")
            set(QT_SDK_DIR "$ENV{QT_ROOT_DIR}" CACHE PATH "Qt SDK directory")
        elseif(WIN32)
            set(QT_SDK_DIR "D:/software/Qt6.8.3/6.8.3/msvc2022_64" CACHE PATH "Qt SDK directory")
        elseif(APPLE)
            set(QT_SDK_DIR "/Users/ze/Qt/6.10.1/macos" CACHE PATH "Qt SDK directory")
        endif()
    endif()

    if(QT_SDK_DIR)
        list(PREPEND CMAKE_PREFIX_PATH "${QT_SDK_DIR}")
    endif()

    find_package(QT NAMES Qt6 Qt5 REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

    if(QT_VERSION_MAJOR LESS 6)
        message(FATAL_ERROR "QScss first version requires Qt 6 or newer.")
    endif()
endif()

if(QSCSS_BUILD_QT_WIDGETS_ADAPTER)
    add_library(QScssQtWidgets
        src/QtWidgetsStyleAdapter.cpp
        include/QScss/QtWidgetsStyleAdapter.h
    )

    add_library(QScss::QtWidgets ALIAS QScssQtWidgets)

    target_compile_features(QScssQtWidgets PUBLIC cxx_std_17)

    target_include_directories(QScssQtWidgets
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(QScssQtWidgets
        PUBLIC
            QScss::Core
            Qt${QT_VERSION_MAJOR}::Widgets
    )
endif()

if(QSCSS_BUILD_EXAMPLES)
    if(NOT QSCSS_BUILD_QT_WIDGETS_ADAPTER)
        message(FATAL_ERROR "QSCSS_BUILD_EXAMPLES requires QSCSS_BUILD_QT_WIDGETS_ADAPTER.")
    endif()

    add_subdirectory(examples/widgets)
endif()
