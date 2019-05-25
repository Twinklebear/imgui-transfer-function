cmake_minimum_required(VERSION 3.5)
project(imgui-tfn)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

add_subdirectory(example)
add_subdirectory(util)

