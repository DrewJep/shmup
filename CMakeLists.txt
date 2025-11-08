cmake_minimum_required(VERSION 3.15)
project(Shmup VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
# Set module path for FindSFML (use our custom FindSFML.cmake)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

# Try to find SFML via Homebrew on macOS first
if(APPLE)
    execute_process(
        COMMAND brew --prefix sfml
        OUTPUT_VARIABLE SFML_HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(SFML_HOMEBREW_PREFIX AND EXISTS "${SFML_HOMEBREW_PREFIX}")
        set(CMAKE_PREFIX_PATH "${SFML_HOMEBREW_PREFIX}" ${CMAKE_PREFIX_PATH})
        set(SFML_ROOT "${SFML_HOMEBREW_PREFIX}")
    endif()
endif()

# Find SFML package (using our custom FindSFML.cmake)
# Note: This project uses SFML 3.0 API
find_package(SFML REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIRS})

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_LIBRARIES})

# On macOS, we may need to link additional frameworks
if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CARBON_FRAMEWORK Carbon)
    if(COCOA_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_FRAMEWORK})
    endif()
    if(IOKIT_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${IOKIT_FRAMEWORK})
    endif()
    if(CARBON_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${CARBON_FRAMEWORK})
    endif()
endif()

# Copy assets to build directory (for development)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR} 
     FILES_MATCHING PATTERN "*" 
     PATTERN ".gitkeep" EXCLUDE)

# Build configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif()

