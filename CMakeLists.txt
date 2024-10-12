cmake_minimum_required(VERSION 3.29)

# Other project need *some way to include this stuff easier.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
if (DEFINED PROJECT_NAME)
  set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endif()

project(mj LANGUAGES C CXX)

include(mj)

add_library(${PROJECT_NAME} STATIC
"include/mj/mj.hpp"
"src/mj.cpp"
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_include_directories(${PROJECT_NAME} PUBLIC "include")
MJFullCompilerWarnings(${PROJECT_NAME})