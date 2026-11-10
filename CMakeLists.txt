cmake_minimum_required(VERSION 3.5)

project(Vycaz VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg OpenGLWidgets)

set(TS_FILES Vycaz_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Vycaz
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        graphicsview.h graphicsview.cpp
        machineitem.h machineitem.cpp
        graphicscene.h graphicscene.cpp
        factorybutton.h factorybutton.cpp
        portitem.h portitem.cpp
        connectionitem.h connectionitem.cpp
        connectiongraph.h
        machinedefinition.h
        machinelibrary.h machinelibrary.cpp


        portdefinition.h
        machineconfiguredialog.h machineconfiguredialog.cpp machineconfiguredialog.ui

        recipedefinition.h

        materialdefinition.h
        portconfigitem.h portconfigitem.cpp
        containeritem.h containeritem.cpp
        colorlabel.h colorlabel.cpp
        preferencesdialog.h preferencesdialog.cpp preferencesdialog.ui
        preferencesdefinition.h
        settingsdefinition.h
        data.qrc

    )
endif()

target_include_directories(Vycaz PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Vycaz PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::OpenGLWidgets)

set_target_properties(Vycaz PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Vycaz
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Vycaz)
endif()
