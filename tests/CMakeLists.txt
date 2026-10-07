#CPMAddPackage("gh:catchorg/Catch2@3.16.0")
find_package(Catch2 REQUIRED)
#list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
#message("${catch2_SOURCE_DIR} aaa")
include(CTest)
include(Catch)

add_executable(${PROJECT_NAME}_test
    "algorithm_test.cpp"
    "math_test.cpp"
    "nameof_test.cpp"
    "pool_test.cpp"
    "size_test.cpp"
    "smolvector_test.cpp"
    "helpers.hpp"
)
target_compile_features(${PROJECT_NAME}_test PUBLIC cxx_std_23)
target_link_libraries(${PROJECT_NAME}_test PUBLIC ${PROJECT_NAME} Catch2::Catch2WithMain)
MJFullCompilerWarnings(${PROJECT_NAME}_test)
MJSanitize(${PROJECT_NAME}_test ON ON ON OFF OFF)

enable_testing()
#add_test(NAME mj_test COMMAND "$<TARGET_FILE:mj_test>")
catch_discover_tests(mj_test)
