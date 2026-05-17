cmake_minimum_required(VERSION 3.16)
project(Stage5_Polling LANGUAGES CXX)

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

# 构建后自动部署 Qt DLL
find_program(WINDEPLOYQT_EXECUTABLE windeployqt
    HINTS "${CMAKE_PREFIX_PATH}/bin" "${_qt_bin_dir}")
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt DLLs..."
    )
endif()
