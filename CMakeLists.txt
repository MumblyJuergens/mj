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

set(mj_source 
"src/mj.cpp"
)
set(mj_headers 
"include/mj/lambda.hpp"
"include/mj/mj.hpp"
"include/mj/nameof.hpp"
"include/mj/pool.hpp"
"${CMAKE_CURRENT_BINARY_DIR}/include/mj/${PROJECT_NAME}_export.h"
)
add_library(${PROJECT_NAME} ${mj_source} ${mj_headers})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
MJFullCompilerWarnings(${PROJECT_NAME})
MJLibrarySetup(${PROJECT_NAME} INCLUDE_DIR include EXPORT_HEADER_PREFIX "mj/mj" HEADERS ${mj_headers})

# I have no clue if this is correct...
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
if (NOT_SUBPROJECT)
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/mjConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/mjConfig.cmake
        INSTALL_DESTINATION  "${CMAKE_INSTALL_LIBDIR}/cmake/mj"
    )
    install(
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mjConfig.cmake"
      DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    )

    add_subdirectory("tests")
endif()