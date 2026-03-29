cmake_minimum_required(VERSION 3.19)
project(markovreliabilitymodel LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(markovreliabilitymodel
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    node.h
    nodegraphics.h
    reliabilityscene.h
    statescene.h
    node.cpp
    nodegraphics.cpp
    reliabilityscene.cpp
    statescene.cpp
    сonnection.h
    сonnectiongraphics.h
    connection.cpp
    connectiongraphics.cpp



)

target_link_libraries(markovreliabilitymodel
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS markovreliabilitymodel
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET markovreliabilitymodel
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
