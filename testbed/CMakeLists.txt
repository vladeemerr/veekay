cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(testbed LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED TRUE CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} veekay Vulkan::Headers)
