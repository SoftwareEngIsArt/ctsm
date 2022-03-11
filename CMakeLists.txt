cmake_minimum_required(VERSION 3.22)
project(ctsm)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} INTERFACE)

include(${CMAKE_CURRENT_LIST_DIR}/ctsm/CMakeLists.txt)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
