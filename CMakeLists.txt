cmake_minimum_required(VERSION 3.0)
project(renderer)

set(SOURCES
    src/main.cc
    src/linalg.hh
    src/codegen/surface.h
    src/codegen/surface.c
)

add_executable(renderer ${SOURCES})

set_target_properties(renderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add compile options
target_compile_options(renderer PUBLIC -Wall -Wextra -Wfloat-conversion)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
