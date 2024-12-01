cmake_minimum_required(VERSION 3.29)

# Are we bundled...
if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
else()
  set(NOT_SUBPROJECT OFF)
endif()

# If we are used as a subdirectory...
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
if (NOT NOT_SUBPROJECT)
  set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endif()

project(mj  VERSION 0.0.1
            DESCRIPTION "C++ and CMake helper library"
            LANGUAGES CXX)
include(mj)

include("cmake/CPM.cmake")
CPMAddPackage("gh:microsoft/GSL#main")

set(mj_source 
"src/mj.cpp"
)
set(mj_headers 
"include/mj/algorithm.hpp"
"include/mj/console.hpp"
"include/mj/iterator.hpp"
"include/mj/lambda.hpp"
"include/mj/math.hpp"
"include/mj/mj.hpp"
"include/mj/nameof.hpp"
"include/mj/pool.hpp"
"include/mj/size.hpp"
"include/mj/string.hpp"
"include/mj/timer.hpp"
"${CMAKE_CURRENT_BINARY_DIR}/include/mj/${PROJECT_NAME}_export.h"
)
add_library(mj ${mj_source} ${mj_headers})
add_library(mj::mj ALIAS mj)
target_compile_features(mj PUBLIC cxx_std_23)
target_link_libraries(mj PUBLIC Microsoft.GSL::GSL)
MJFullCompilerWarnings(mj)
MJLibrarySetup(mj INCLUDE_DIR "include" EXPORT_HEADER_PREFIX "mj/mj" HEADERS ${mj_headers})

if (NOT_SUBPROJECT)
    MJLibraryInstallSetup(mj)
    add_subdirectory("tests")
endif()