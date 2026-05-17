cmake_minimum_required(VERSION 3.16)
project(Stage13_JsonConfig LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(${PROJECT_NAME}
    main.cpp
    testwindow.h        testwindow.cpp
    config_parser.h     config_parser.cpp
    widget_factory.h    widget_factory.cpp
    data_binding.h      data_binding.cpp
    screen_builder.h    screen_builder.cpp
    sim_data_manager.h  sim_data_manager.cpp
    nav_bar.h           nav_bar.cpp
    tank_widget.h       tank_widget.cpp
    pump_widget.h       pump_widget.cpp
    valve_widget.h      valve_widget.cpp
    pipe_widget.h       pipe_widget.cpp
    value_display.h     value_display.cpp
    resources/stage13.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
