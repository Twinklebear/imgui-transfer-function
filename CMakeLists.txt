cmake_minimum_required(VERSION 3.5)
project(imgui-tfn)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

add_definitions(-DNOMINMAX -DSDL_MAIN_HANDLED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(imgui)

add_executable(imgui_tfn main.cpp
	imgui_impl_opengl3.cpp imgui_impl_sdl.cpp gl_core_4_5.c)

set_target_properties(imgui_tfn PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

target_include_directories(imgui_tfn PUBLIC
	$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${OPENGL_INCLUDE_DIR}>)

target_link_libraries(imgui_tfn PUBLIC
	imgui ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})

