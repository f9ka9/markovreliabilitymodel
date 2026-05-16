cmake_minimum_required(VERSION 3.19)
project(markovreliabilitymodel LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(markovreliabilitymodel
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/nodeconfigurationwidget.cpp
    src/ui/nodeconfigurationwidget.h
    src/ui/statescene.cpp
    src/ui/statescene.h
    src/ui/stateview.cpp
    src/ui/stateview.h
    src/ui/structureview.cpp
    src/ui/structureview.h
    src/editor/reliabilityscene.cpp
    src/editor/reliabilityscene.h
    src/editor/nodegraphics.cpp
    src/editor/nodegraphics.h
    src/editor/lineconnectiongraphics.cpp
    src/editor/lineconnectiongraphics.h
    src/editor/gridsettings.h
    src/model/node.cpp
    src/model/node.h
    src/model/lineconnection.cpp
    src/model/lineconnection.h
    src/model/editorschemamodel.h
    src/model/schemamodel.h
    src/model/cyclogram.h
    src/serialization/schemaserializer.cpp
    src/serialization/schemaserializer.h
    src/core/reliabilitystate.h
    src/core/stategenerator.cpp
    src/core/stategenerator.h
    src/core/transitionbuilder.cpp
    src/core/transitionbuilder.h
    src/core/matrixbuilder.cpp
    src/core/matrixbuilder.h
    src/core/probabilitycalculator.cpp
    src/core/probabilitycalculator.h
    src/core/compositereliabilitycalculator.cpp
    src/core/compositereliabilitycalculator.h
    src/core/reliabilitycore.cpp
    src/core/reliabilitycore.h
    src/core/calculationresult.h
)

target_include_directories(markovreliabilitymodel
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/editor
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/serialization
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
