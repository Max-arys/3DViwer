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
    model/model.cpp
    model/transform.cpp
    model/display_settings.cpp
    controller/wcontroller.cpp
    view/mainwindow.cpp
    view/glwidget.cpp
    view/imagesaver.cpp
    view/gif_recorder.cpp
    view/main.cpp
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
