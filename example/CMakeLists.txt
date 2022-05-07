add_definitions(-DNOMINMAX -DSDL_MAIN_HANDLED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(imgui)

add_executable(imgui_tfn
    main.cpp
    ../transfer_function_widget.cpp
    shader.cpp
	imgui_impl_opengl3.cpp
    imgui_impl_sdl.cpp
    ../gl_core_4_5.c)

set_target_properties(imgui_tfn PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

target_include_directories(imgui_tfn PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${OPENGL_INCLUDE_DIR}>)

target_link_libraries(imgui_tfn PUBLIC
	imgui ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})

target_compile_definitions(imgui_tfn PUBLIC
    -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM)

