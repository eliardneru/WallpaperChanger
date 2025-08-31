cmake_minimum_required(VERSION 3.16)

project(WallpaperChanger VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Tell AUTOUIC where to search for .ui files
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

set(PROJECT_SOURCES
    src/main.cpp
    src/wallpaperchanger.cpp
    src/wallpaperchanger.h
    forms/wallpaperchanger.ui
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WallpaperChanger
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
        .gitignore
        CHANGELOG
        icons.rc
    )
else()
    if(ANDROID)
        add_library(WallpaperChanger SHARED ${PROJECT_SOURCES})
    else()
        add_executable(WallpaperChanger ${PROJECT_SOURCES})
    endif()
endif()

target_include_directories(WallpaperChanger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

# Link Qt libraries
target_link_libraries(WallpaperChanger PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Statically compile it
target_compile_options(WallpaperChanger PRIVATE -static)

# Properties for macOS / Windows
set_target_properties(WallpaperChanger PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Installation rules
include(GNUInstallDirs)
install(TARGETS WallpaperChanger
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalize for Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WallpaperChanger)
endif()
