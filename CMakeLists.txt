cmake_minimum_required(VERSION 3.14)

set(MODULE async_issue)

project(${MODULE}
    LANGUAGES C CXX
    DESCRIPTION "Project for reproduce the issue"
)

find_package(Threads REQUIRED)

add_subdirectory(pybind11)

file(GLOB_RECURSE MODULE_SRC src/*.cpp)

pybind11_add_module(${MODULE} ${MODULE_SRC})

set_property(TARGET ${MODULE} PROPERTY CXX_STANDARD 14)

target_link_libraries(${MODULE} PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
)