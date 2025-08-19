cmake_minimum_required(VERSION 3.20)

project(ccwc
    VERSION 0.1.1
    DESCRIPTION "High-performance word count tool"
    LANGUAGES CXX
)

# Enforce modern standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# collect all source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/main.cpp
)

add_executable(ccwc ${SOURCES})

target_include_directories(ccwc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
        -Wold-style-cast -Woverloaded-virtual -Wnon-virtual-dtor
        -Wnull-dereference -Wdouble-promotion -Wformat=2
    )
elseif (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
