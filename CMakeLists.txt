cmake_minimum_required(VERSION 3.15)
project(Shmup VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

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

find_package(SFML REQUIRED)


# Try pkg-config first
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_search_module(JSONCPP jsoncpp)
endif()

if (NOT JSONCPP_FOUND OR NOT JSONCPP_LIBRARIES OR NOT JSONCPP_INCLUDE_DIRS)
    message(STATUS "jsoncpp not found via pkg-config; trying fallbacks...")

    # 1) Try Homebrew prefix (useful on macOS)
    if (APPLE)
        execute_process(
            COMMAND brew --prefix jsoncpp
            OUTPUT_VARIABLE JSONCPP_HOMEBREW_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if (JSONCPP_HOMEBREW_PREFIX)
            # headers may be installed as <json/json.h> under include/
            if (EXISTS "${JSONCPP_HOMEBREW_PREFIX}/include/json/json.h")
                set(JSONCPP_INCLUDE_DIRS "${JSONCPP_HOMEBREW_PREFIX}/include")
            elseif (EXISTS "${JSONCPP_HOMEBREW_PREFIX}/include/jsoncpp/json/json.h")
                set(JSONCPP_INCLUDE_DIRS "${JSONCPP_HOMEBREW_PREFIX}/include/jsoncpp")
            endif()
            if (EXISTS "${JSONCPP_HOMEBREW_PREFIX}/lib/libjsoncpp.dylib")
                set(JSONCPP_LIBRARIES "${JSONCPP_HOMEBREW_PREFIX}/lib/libjsoncpp.dylib")
            elseif (EXISTS "${JSONCPP_HOMEBREW_PREFIX}/lib/libjsoncpp.a")
                set(JSONCPP_LIBRARIES "${JSONCPP_HOMEBREW_PREFIX}/lib/libjsoncpp.a")
            endif()
        endif()
    endif()

    if (NOT JSONCPP_INCLUDE_DIRS)
        find_path(JSONCPP_INCLUDE_DIR
                  NAMES json/json.h
                  PATHS /opt/homebrew/include /usr/local/include /usr/include
                  PATH_SUFFIXES json jsoncpp NO_DEFAULT_PATH)
        # If not found in the restricted paths, try default search paths too
        if (NOT JSONCPP_INCLUDE_DIR)
            find_path(JSONCPP_INCLUDE_DIR NAMES json/json.h PATH_SUFFIXES json jsoncpp)
        endif()
        if (JSONCPP_INCLUDE_DIR)
            set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
        endif()
    endif()

    if (NOT JSONCPP_LIBRARIES)
        find_library(JSONCPP_LIBRARY
                     NAMES jsoncpp jsoncpp_lib
                     PATHS /opt/homebrew/lib /usr/local/lib /usr/lib
                     NO_DEFAULT_PATH)
        if (NOT JSONCPP_LIBRARY)
            find_library(JSONCPP_LIBRARY NAMES jsoncpp jsoncpp_lib)
        endif()
        if (JSONCPP_LIBRARY)
            set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
        endif()
    endif()
endif()

if (NOT JSONCPP_LIBRARIES)
    message(FATAL_ERROR "jsoncpp not found! Install with: brew install jsoncpp")
endif()

# Prefer a concrete .dylib/.so/.a so linking works without setting LIBRARY_PATH (e.g. macOS + Homebrew).
if (NOT EXISTS "${JSONCPP_LIBRARIES}")
    find_library(JSONCPP_LIBRARY_ABSOLUTE NAMES jsoncpp jsoncpp_lib
        PATHS
            "${JSONCPP_HOMEBREW_PREFIX}/lib"
            /opt/homebrew/lib
            /usr/local/lib
            /usr/lib
        NO_DEFAULT_PATH)
    if (NOT JSONCPP_LIBRARY_ABSOLUTE)
        find_library(JSONCPP_LIBRARY_ABSOLUTE NAMES jsoncpp jsoncpp_lib)
    endif()
    if (JSONCPP_LIBRARY_ABSOLUTE)
        set(JSONCPP_LIBRARIES "${JSONCPP_LIBRARY_ABSOLUTE}")
    endif()
endif()

message(STATUS "jsoncpp include dirs: ${JSONCPP_INCLUDE_DIRS}")
message(STATUS "jsoncpp libs: ${JSONCPP_LIBRARIES}")


file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SFML_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${JSONCPP_INCLUDE_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFML_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${JSONCPP_LIBRARIES})

# macOS frameworks
if(APPLE)
    foreach(FW Cocoa IOKit Carbon)
        find_library(${FW}_FRAMEWORK ${FW})
        if(${FW}_FRAMEWORK)
            target_link_libraries(${PROJECT_NAME} PRIVATE ${${FW}_FRAMEWORK})
        endif()
    endforeach()
endif()

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}
     FILES_MATCHING PATTERN "*"
     PATTERN ".gitkeep" EXCLUDE)

# Debug build setup
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif()
