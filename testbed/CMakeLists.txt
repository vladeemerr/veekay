cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(testbed LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED TRUE CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} veekay Vulkan::Headers)

# Compile shaders
find_program(GLSLC_FOUND glslc)
if(GLSLC_FOUND)
	set(_SHADER_BINARIES)

	macro(compile_shader SHADER_FILE)
		set(SHADER_SOURCE ${CMAKE_SOURCE_DIR}/shaders/${SHADER_FILE})
		set(SHADER_BINARY ${SHADER_FILE}.spv)
		set(SHADER_BINARY_PATH ${CMAKE_SOURCE_DIR}/shaders/${SHADER_BINARY})

		add_custom_command(
			OUTPUT ${SHADER_BINARY_PATH}
			COMMAND glslc ${SHADER_SOURCE} -o ${SHADER_BINARY_PATH}
			DEPENDS ${SHADER_SOURCE}
			COMMENT "Compiling ${SHADER_FILE} shader"
		)

		list(APPEND _SHADER_BINARIES ${SHADER_BINARY_PATH})
	endmacro()

	# To compile shader file, use compile_shader function with a file name
	# of a shader inside shaders directory. See example below

	compile_shader(shader.vert)
	compile_shader(shader.frag)

	add_custom_target(shaders DEPENDS ${_SHADER_BINARIES})
	add_dependencies(${PROJECT_NAME} shaders)
endif()
