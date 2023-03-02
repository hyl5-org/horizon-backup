cmake_minimum_required(VERSION 3.18)

project(Horizon)
include(C:/FILES/vcpkg/scripts/buildsystems/vcpkg.cmake)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/horizon)
set(SOLUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# cpp tool chain
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-format-style='file';--extra-arg-before=-std=c++17;${CLANG_TIDY_ARGS}")
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE include-what-you-use)

add_subdirectory(horizon/3rd_party)
add_subdirectory(horizon/runtime)
add_subdirectory(horizon/unit_tests)
add_subdirectory(horizon/samples)
