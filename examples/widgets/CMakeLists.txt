qt_add_executable(qscss_widgets_example
    main.cpp
    resources.qrc
)

target_link_libraries(qscss_widgets_example
    PRIVATE
        QScss::QtWidgets
)

target_compile_definitions(qscss_widgets_example
    PRIVATE
        QSCSS_DIST_DIR="${CMAKE_SOURCE_DIR}/dist/qss"
)

if(TARGET qscss_build_styles)
    add_dependencies(qscss_widgets_example qscss_build_styles)
endif()
