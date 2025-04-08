cmake_minimum_required(VERSION 3.30)
project(ccwc_in_cpp)

set(CMAKE_CXX_STANDARD 20)

# Add Custom warnings from Clang - http://tinyurl.com/ClangDiagRef
set(CUSTOM_CXX_FLAG_WALL "-Wall")
set(CUSTOM_CXX_FLAG_TURN_WARN_TO_ERR "-Werror")
set(CUSTOM_CXX_FLAG_WARN_PEDANTIC "-Wpedantic")
set(CUSTOM_CXX_FLAG_PEDANTIC_ERR "-pedantic-errors")
set(CUSTOM_CXX_FLAG_WARN_EXTRA "-Wextra")
set(CUSTOM_CXX_FLAG_EFFECTIVE_CXX_ERR "-Weffc++")
set(CUSTOM_CXX_FLAG_CONVERSION_WARN "-Wconversion")
set(CUSTOM_CXX_FLAG_SIGN_CONVERSION_WARN "-Wsign-conversion")

# Combine errors
set(CUSTOM_WARN_ERRORS "\
${CUSTOM_CXX_FLAG_WALL} \
${CUSTOM_CXX_FLAG_TURN_WARN_TO_ERR} \
${CUSTOM_CXX_FLAG_WARN_PEDANTIC} \
${CUSTOM_CXX_FLAG_PEDANTIC_ERR} \
${CUSTOM_CXX_FLAG_WARN_EXTRA} \
${CUSTOM_CXX_FLAG_EFFECTIVE_CXX_ERR} \
${CUSTOM_CXX_FLAG_CONVERSION_WARN} \
${CUSTOM_CXX_FLAG_SIGN_CONVERSION_WARN}")

# Print warnings
message(CUSTOM_WARNINGS="${CUSTOM_WARN_ERRORS}")

set(CMAKE_CXX_FLAGS "${CUSTOM_WARN_ERRORS}")

# Add dependency: quill
include(FetchContent)
FetchContent_Declare(
        quill
        GIT_REPOSITORY https://github.com/odygrd/quill.git
        GIT_TAG bfc2f92f9a7203910a05f7d46e6d4967adc00f69  # v9.0.0
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/quill
)
FetchContent_MakeAvailable(quill)

# Add Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(ccwc_in_cpp ${SOURCES})

# Add include directories
target_include_directories(ccwc_in_cpp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ccwc_in_cpp PRIVATE quill::quill)
