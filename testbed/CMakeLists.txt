cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(testbed LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} veekay)
