CPMAddPackage("gh:catchorg/Catch2@3.7.1")

add_executable(${PROJECT_NAME}_tests
"nameof_tests.cpp"
"pool_tests.cpp"
)
target_compile_features(${PROJECT_NAME}_tests PUBLIC cxx_std_23)
target_link_libraries(${PROJECT_NAME}_tests PUBLIC ${PROJECT_NAME} Catch2::Catch2WithMain)
MJFullCompilerWarnings(${PROJECT_NAME}_tests)

enable_testing()
add_test(${PROJECT_NAME}_tests ${PROJECT_NAME}_tests)