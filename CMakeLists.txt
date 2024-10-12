cmake_minimum_required(VERSION 3.29)

project(mj LANGUAGES C CXX)

include("cmake/Mumbly.cmake")

add_library(${PROJECT_NAME} STATIC
"include/mj/mj.hpp"
"src/mj.cpp"
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_include_directories(${PROJECT_NAME} PUBLIC "include")