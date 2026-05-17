cmake_minimum_required(VERSION 3.16)
project(Stage4_WriteData LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    plc_worker.cpp
    snap7.cpp
)

target_link_options(${PROJECT_NAME} PRIVATE -static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    ${CMAKE_SOURCE_DIR}/libsnap7_mingw.a
    ws2_32
)
