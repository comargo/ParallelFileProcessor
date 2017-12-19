cmake_minimum_required(VERSION 3.9)
project(ParallelFileProcessor)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Qt5 COMPONENTS Widgets)

set(SOURCES 
	main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        controller.cpp
        controller.h
	)
add_executable(${PROJECT_NAME} ${SOURCES})
qt5_use_modules(${PROJECT_NAME} Widgets)
