set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
cmake_minimum_required(VERSION 3.10)

project(partitioner)

# import general dependencies
find_package(Threads REQUIRED)

# import Qt5 dependencies
set(QT_VERSION_REQ "5.2")
find_package(Qt5Core ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Gui ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Widgets ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Svg ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Test ${QT_VERSION_REQ} REQUIRED)
find_package(Qt5Charts ${QT_VERSION_REQ} REQUIRED)

# general settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_testing()

# add resources
qt5_add_resources(CUSTOM_RSC qrc/application.qrc)

# source and header files
set(LIB_SOURCES
    spatial.cc
    partitioner/partitioner.cc
    gui/settings.cc
    gui/mainwindow.cc
    gui/dtviewer.cc
    gui/partviewer.cc
    gui/invoker.cc
    gui/telemetrychart.cc
    gui/prim/graph_mask.cc
    gui/prim/net.cc
    gui/prim/cell.cc
    #    gui/prim/cell.cc
    #    gui/prim/net.cc
    )
set(LIB_HEADERS
    spatial.h
    partitioner/partitioner.h
    gui/settings.h
    gui/mainwindow.h
    gui/dtviewer.h
    gui/partviewer.h
    gui/invoker.h
    gui/telemetrychart.h
    gui/prim/graph_mask.h
    gui/prim/net.h
    gui/prim/cell.h
    #    gui/invoker.h
    #    gui/prim/cell.h
    #    gui/prim/net.h
    )

# libraries to be linked
set(LIB_LINKS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    Qt5::Charts
    )

# inclusions
include_directories(.)

# build application
add_executable(partitioner MACOSX_BUNDLE main.cc ${LIB_SOURCES} ${LIB_HEADERS} ${CUSTOM_RSC})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIB_LINKS} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(partitioner PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER my.domain.style.identifier.partitioner
    MACOSX_BUNDLE_BUNDLE_NAME partitioner
    MACOSX_BUNDLE_BUNDLE_VERSION "0.0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.1"
)

# build unit tests
add_executable(partitioner_tests tests/partitioner_tests.cpp ${LIB_SOURCES} ${LIB_HEADERS} ${CUSTOM_RSC})
target_link_libraries(partitioner_tests Qt5::Test ${LIB_LINKS} ${CMAKE_THREAD_LIBS_INIT})
add_test(partitioner_tests partitioner_tests)
set_tests_properties(partitioner_tests PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
add_custom_command(TARGET partitioner_tests
    POST_BUILD
    COMMAND ctest -C $<CONFIGURATION> --output-on-failure)

# install the binary
install(TARGETS partitioner
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
