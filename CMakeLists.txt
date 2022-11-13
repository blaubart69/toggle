cmake_minimum_required(VERSION 3.0.0)
project(toggle VERSION 0.1.0)

include(CTest)
enable_testing()

set(SOURCES 
    main.cpp 
    auto_reset_event.cpp
    auto_reset_event.hpp
    Postbox.hpp
    Postbox.cpp
    ResourcePipeline.cpp
    ResourcePipeline.hpp
)

add_executable(toggle ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
