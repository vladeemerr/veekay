cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(veekay LANGUAGES C CXX)

add_library(${PROJECT_NAME} source/veekay.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${veekay_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(LINUX)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
elseif(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
	target_link_options(${PROJECT_NAME} PRIVATE /subsystem:windows /entry:mainCRTStartup)
else()
	message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED TRUE CXX_STANDARD 20)

add_subdirectory(testbed)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
)

FetchContent_Declare(
	vk-bootstrap
	GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
	GIT_TAG v1.4.321
)

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui
	GIT_TAG v1.92.3
)

find_package(Vulkan REQUIRED)

set(GLFW_LIBRARY_TYPE STATIC)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

FetchContent_MakeAvailable(glfw vk-bootstrap imgui)

target_link_libraries(${PROJECT_NAME} PRIVATE
	glfw
	Vulkan::Vulkan
	vk-bootstrap::vk-bootstrap
)

# Link ImGui
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_sources(${PROJECT_NAME} PRIVATE
	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

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

	compile_shader(shader.vert)
	compile_shader(shader.frag)

	add_custom_target(shaders DEPENDS ${_SHADER_BINARIES})
	add_dependencies(veekay shaders)
endif()
