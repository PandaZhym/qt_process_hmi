cmake_minimum_required(VERSION 3.16)
project(Stage6_ProcessHMI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# snap7 C++ wrapper
add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    plc_worker.cpp
    plc_worker.h
    snap7.cpp
    snap7.h
    ../process_widgets/tank_widget.cpp
    ../process_widgets/tank_widget.h
    ../process_widgets/pump_widget.cpp
    ../process_widgets/pump_widget.h
    ../process_widgets/valve_widget.cpp
    ../process_widgets/valve_widget.h
    ../process_widgets/pipe_widget.cpp
    ../process_widgets/pipe_widget.h
    ../process_widgets/value_display.cpp
    ../process_widgets/value_display.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ../process_widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/libsnap7_mingw.a ws2_32)
target_link_options(${PROJECT_NAME} PRIVATE -static)

# 构建后自动部署 Qt DLL
find_program(WINDEPLOYQT_EXECUTABLE windeployqt
    HINTS "${CMAKE_PREFIX_PATH}/bin")
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt DLLs..."
    )
endif()
