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

project(mj LANGUAGES C CXX)

include(mj)

add_library(${PROJECT_NAME} STATIC
"include/mj/lambda.hpp"
"include/mj/mj.hpp"
"src/mj.cpp"
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_include_directories(${PROJECT_NAME} PUBLIC "include")
MJFullCompilerWarnings(${PROJECT_NAME})

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
endif()