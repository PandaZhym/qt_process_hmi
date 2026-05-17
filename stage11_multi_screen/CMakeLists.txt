cmake_minimum_required(VERSION 3.16)
project(Stage11_Multi_Screen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(${PROJECT_NAME}
    main.cpp
    hmi_screen.h
    testwindow.h           testwindow.cpp
    nav_bar.h              nav_bar.cpp
    sim_data_manager.h     sim_data_manager.cpp
    screen_overview.h      screen_overview.cpp
    screen_process.h       screen_process.cpp
    screen_alarm.h         screen_alarm.cpp
    screen_trend.h         screen_trend.cpp
    screen_settings.h      screen_settings.cpp
    tank_widget.h          tank_widget.cpp
    pump_widget.h          pump_widget.cpp
    valve_widget.h         valve_widget.cpp
    pipe_widget.h          pipe_widget.cpp
    value_display.h        value_display.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
