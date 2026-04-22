cmake_minimum_required(VERSION 3.16)

if(NOT PROJECT_NAME)
    set(PROJECT_NAME "3DViewer")
endif()

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4.2 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
find_package(GIF REQUIRED)

qt6_add_executable(${PROJECT_NAME}
    src/model/model.cpp
    src/model/transform.cpp
    src/model/display_settings.cpp
    src/controller/wcontroller.cpp
    src/view/mainwindow.cpp
    src/view/glwidget.cpp
    src/view/imagesaver.cpp
    src/view/gif_recorder.cpp
    src/view/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
    assimp::assimp
    ${GIF_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
