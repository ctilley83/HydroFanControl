cmake_minimum_required(VERSION 3.19)
project(HydroFanControl LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui)

qt_standard_project_setup()

qt_add_executable(HydroFanControl
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ledmode.h ledmode.cpp
)

# Manually specify HIDAPI paths
set(HIDAPI_INCLUDE_DIR /usr/include/hidapi)
set(HIDAPI_LIBRARY /usr/lib/libhidapi-libusb.so)  # Use libusb backend
target_include_directories(HydroFanControl PRIVATE ${HIDAPI_INCLUDE_DIR})
target_link_libraries(HydroFanControl PRIVATE ${HIDAPI_LIBRARY} Qt::Core Qt::Widgets)

include(GNUInstallDirs)

install(TARGETS HydroFanControl
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET HydroFanControl
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
