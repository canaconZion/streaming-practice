# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
# cmake_minimum_required(VERSION 3.16.3)

# # Set the project name
# project (twoSum)

# # Add an executable
# add_executable(twoSum test.cpp)
# target_link_libraries(lgtest lpthread)

cmake_minimum_required(VERSION 3.16.3)
project(test)
find_package (Threads)
include_directories(/user/include)
link_directories(/usr/lib)
add_executable(test test.cpp)
target_link_libraries(test gtest gtest_main)
target_link_libraries(test ${CMAKE_THREAD_LIBS_INIT})