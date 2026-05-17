cmake_minimum_required(VERSION 3.16)
project(Stage10_Trend_Sqlite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(Stage10_Trend_Sqlite
    main.cpp
    testwindow.h    testwindow.cpp
    trend_chart.h   trend_chart.cpp
    data_logger.h   data_logger.cpp
)

target_link_libraries(Stage10_Trend_Sqlite PRIVATE
    Qt6::Widgets
    Qt6::Sql
)
