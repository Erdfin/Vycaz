cmake_minimum_required(VERSION 3.24)

project(Vycaz VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg OpenGLWidgets)

# ---------------------------------------------------------------------------
# MicroTeX
# ---------------------------------------------------------------------------
include(FetchContent)

# tinyxml2 is used internally by MicroTeX to load its XML font/symbol tables.
set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG        9.0.0
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(tinyxml2)

# Pin MicroTeX for reproducible builds. SOURCE_SUBDIR names a directory with no
# CMakeLists.txt on purpose: FetchContent only downloads the sources; this
# project compiles MicroTeX itself because upstream's CMake always builds its
# own Qt sample and expects an installed tinyxml2 package.
FetchContent_Declare(
    microtex
    GIT_REPOSITORY https://github.com/NanoMichael/MicroTeX.git
    GIT_TAG        0e3707f6dafebb121d98b53c64364d16fefe481d
    GIT_SHALLOW    FALSE
    GIT_SUBMODULES ""
    SOURCE_SUBDIR  cmake/populate-only
)
FetchContent_MakeAvailable(microtex)

set(MICROTEX_SRC "${microtex_SOURCE_DIR}/src")

file(GLOB_RECURSE MICROTEX_SOURCES
    "${MICROTEX_SRC}/atom/*.cpp"
    "${MICROTEX_SRC}/box/*.cpp"
    "${MICROTEX_SRC}/core/*.cpp"
    "${MICROTEX_SRC}/fonts/*.cpp"
    "${MICROTEX_SRC}/utils/*.cpp"
    "${MICROTEX_SRC}/res/*.cpp"
    "${MICROTEX_SRC}/latex.cpp"
    "${MICROTEX_SRC}/render.cpp"
)
# Patched copy of upstream's platform/qt/graphic_qt.cpp: upstream draws 1pt
# fonts under a large painter scale, which Qt cannot rasterise.
list(APPEND MICROTEX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/microtex/graphic_qt.cpp")

add_library(microtex STATIC ${MICROTEX_SOURCES})
add_library(MicroTeX::microtex ALIAS microtex)

target_include_directories(microtex PUBLIC "${MICROTEX_SRC}")
target_compile_definitions(microtex PUBLIC BUILD_QT)
target_compile_options(microtex PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
target_link_libraries(microtex
    PUBLIC  Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE tinyxml2::tinyxml2
)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------

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
        microtexruntime.h microtexruntime.cpp
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

target_link_libraries(Vycaz PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    MicroTeX::microtex
)

set_target_properties(Vycaz PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# MicroTeX reads its fonts and XML tables from disk at runtime.
add_custom_command(TARGET Vycaz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${microtex_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:Vycaz>/res"
    COMMENT "Copying MicroTeX resources next to the executable"
    VERBATIM
)

include(GNUInstallDirs)
install(TARGETS Vycaz
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY "${microtex_SOURCE_DIR}/res" DESTINATION .)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Vycaz)
endif()
