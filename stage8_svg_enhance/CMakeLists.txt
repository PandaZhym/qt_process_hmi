cmake_minimum_required(VERSION 3.16)
project(Stage8_SVG_Enhance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

add_executable(${PROJECT_NAME}
    main.cpp
    testwindow.cpp    testwindow.h
    tank_widget.cpp    tank_widget.h
    pump_widget.cpp    pump_widget.h
    valve_widget.cpp   valve_widget.h
    pipe_widget.cpp    pipe_widget.h
    value_display.cpp  value_display.h
    svg_render_helpers.cpp svg_render_helpers.h
    svg_renderer_pool.cpp  svg_renderer_pool.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Svg)
