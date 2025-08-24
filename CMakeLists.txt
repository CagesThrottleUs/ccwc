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
# Source files (.cpp)
set(SOURCES
    src/main.cpp
    src/algorithm/universal_input_stream.cpp
    src/argument_parser/argument_parser.cpp
    src/output_formatter/output_formatter.cpp
)

# Header files (.hpp)
set(HEADERS
    src/algorithm/counter.hpp
    src/algorithm/counter_state_machine.hpp
    src/algorithm/universal_input_stream.hpp
    src/argument_parser/argument_parser.hpp
    src/output_formatter/output_formatter.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

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
# Dependencies
# ============
find_package(Boost REQUIRED COMPONENTS iostreams CONFIG)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::iostreams)

# ============
# Installation
# ============
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Install sample/test.txt next to the binary
# Only install test.txt if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sample/test.txt")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample/test.txt" DESTINATION bin)
else()
    message(WARNING "sample/test.txt not found — skipping its installation")
endif()
