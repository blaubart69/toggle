cmake_minimum_required(VERSION 3.0.0)
project(toggle VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(toggle main.cpp cde.cpp auto_reset_event.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
