cmake_minimum_required(VERSION 3.16)
project(Argos-CRUD LANGUAGES C CXX)  # <-- Añadimos "C" aquí

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(Argos-CRUD
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/sqlite3.c    # Ahora CMake sabrá compilarlo como C nativo
)

target_link_libraries(Argos-CRUD PRIVATE 
    Qt6::Widgets 
    Qt6::Sql
)