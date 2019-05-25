add_executable(embed_image embed_image.cpp)

target_include_directories(embed_image PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

set_target_properties(embed_image PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)


