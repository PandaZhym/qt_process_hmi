cmake_minimum_required(VERSION 3.16)
project(Stage12_Protocol_Abstract LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(${PROJECT_NAME}
    main.cpp
    i_protocol.h
    tag_mapping.h
    simulated_adapter.h   simulated_adapter.cpp
    tag_manager.h          tag_manager.cpp
    value_display.h        value_display.cpp
    testwindow.h           testwindow.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
