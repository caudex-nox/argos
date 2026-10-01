cmake_minimum_required(VERSION 3.16)
project(Argos-CRUD LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

if(WIN32)
    set(WINDOWS_RESOURCES resources/app.rc)
endif()

add_executable(Argos-CRUD
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/AgeSpinBox.h
    src/sqlite3.c
    resources/app.qrc
    ${WINDOWS_RESOURCES}
)

target_link_libraries(Argos-CRUD PRIVATE 
    Qt6::Widgets 
    Qt6::Sql
)