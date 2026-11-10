cmake_minimum_required(VERSION 3.19)
project(heater_gui LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Network)

qt_standard_project_setup()

qt_add_executable(heater_gui
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_translations(
    TARGETS heater_gui
    TS_FILES heater_gui_ru_RU.ts
)

target_link_libraries(heater_gui
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt${QT_VERSION_MAJOR}::Network
)

install(TARGETS heater_gui
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET heater_gui
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
