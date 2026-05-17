cmake_minimum_required(VERSION 3.16)
project(Stage9_Alarm_Mgmt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(${PROJECT_NAME}
    main.cpp
    testwindow.cpp        testwindow.h
    alarm_manager.cpp      alarm_manager.h
    alarm_list_widget.cpp  alarm_list_widget.h
    alarm_banner.cpp       alarm_banner.h
    tag_display.cpp        tag_display.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
