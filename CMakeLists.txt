cmake_minimum_required(VERSION 3.20)

project(ccwc
    VERSION 0.1.1
    DESCRIPTION "High-performance word count tool"
    LANGUAGES CXX
)

# ===============================
# Compiler and Language Standards
# ===============================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always export compile_commands.json (for clangd / tooling)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============
# Sources
# =============
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =================
# Compiler Warnings
# =================
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
        -Wold-style-cast -Woverloaded-virtual -Wnon-virtual-dtor
        -Wnull-dereference -Wdouble-promotion -Wformat=2
    )
elseif (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /EHsc)
endif()

# ======================
# Static Analysis (clang-tidy)
# ======================
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)

    if (CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CLANG_TIDY_COMMAND
            ${CLANG_TIDY_EXE};
            -p=${CMAKE_BINARY_DIR};
            --warnings-as-errors=*;
            --extra-arg-before=--driver-mode=g++
        )
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    else()
        message(WARNING "clang-tidy not found — static analysis disabled")
    endif()
endif()

# ============
# Installation
# ============
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
