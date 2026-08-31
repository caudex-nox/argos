cmake_minimum_required(VERSION 3.16)
project(CrudQtSqlite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(SQLite3 REQUIRED)

add_executable(CrudQtSqlite
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(CrudQtSqlite PRIVATE 
    Qt6::Widgets 
    SQLite::SQLite3
)