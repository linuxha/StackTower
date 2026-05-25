cmake_minimum_required(VERSION 3.16)
project(stacktower_host_tests C)

add_executable(
    stacktower_host_tests
    stacktower_host_tests.c
    ../main/stacktower_core.c
)

target_compile_features(stacktower_host_tests PRIVATE c_std_11)
target_include_directories(stacktower_host_tests PRIVATE ../main)

enable_testing()
add_test(NAME stacktower_host_tests COMMAND stacktower_host_tests)
