cmake_minimum_required(VERSION 3.18)

project(GAMMA_RAY VERSION 0.0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "-Wall -Werror -Wfatal-errors -Wpedantic -Wextra -Wmissing-include-dirs -pthread")
add_compile_options(-Wall -Werror -Wfatal-errors -Wpedantic -Wextra -Wmissing-include-dirs -pthread)


# Options
option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." ON)

# Include
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)
include(CTest)
include(Doctest)
include(Documentation)
include(LTO)
include(Misc)
include(Warnings)
include(Boost)
include(POCO)
include(Websocketpp)
include(Cpprestsdk)

# Check for LTO support.
find_lto(CXX)

# --------------------------------------------------------------------------------
#                         Locate files
# --------------------------------------------------------------------------------
set(SOURCES          # All .cpp files in src/
    src/models.cpp
    src/ws.cpp
    src/bitmexws.cpp
    src/delta_parser.cpp
    src/bitmexhttp.cpp
    src/bitmex_gateway.cpp
    src/market_filtration.cpp
    src/fair_value.cpp
    src/quoting_strategies.cpp
    src/quoting_parameters.cpp
    src/skew.cpp
    src/quoting_engine.cpp
    src/quote_dispatcher.cpp
)
set(TESTFILES        # All .cpp files in tests/
    tests/main.cpp
)
set(LIBRARY_NAME engine)  # Default name for the library built from src/*.cpp

# --------------------------------------------------------------------------------
#                            Build
# --------------------------------------------------------------------------------
# Compile all sources into a library.
add_library(${LIBRARY_NAME} OBJECT ${SOURCES})

# Header files
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${LIBRARY_NAME} PUBLIC doctest)
target_link_libraries(${LIBRARY_NAME} PUBLIC ${Boost_LIBRARIES})
target_link_libraries(${LIBRARY_NAME} PUBLIC websocketpp ssl crypto)
target_link_libraries(${LIBRARY_NAME} PUBLIC Poco::Foundation Poco::Util Poco::Net)
target_link_libraries(${LIBRARY_NAME} PUBLIC cpprestsdk::cpprest)

# Set the compile options
target_set_warnings(${LIBRARY_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
# target_compile_options(${LIBRARY_NAME} ... )  # For setting manually.

# Pre-compiled headers
set(WEBSOCKETPP_DIR "external/websocketpp/websocketpp")
set(DOCTEST_DIR "external/doctest/doctest")
set(POCO_DIR "/usr/local/include/Poco")

target_precompile_headers(${LIBRARY_NAME} PUBLIC 
    <string>
    <vector>
    <map>
    <iostream>
    <stdlib.h>
    <functional>
    <future>
    <chrono>
    <thread>
    <algorithm>
    <cmath>
    <unistd.h>
    "${POCO_DIR}/BasicEvent.h"
    "${POCO_DIR}/Delegate.h"
    "${POCO_DIR}/UUIDGenerator.h"
    "${POCO_DIR}/DateTime.h"
    "${POCO_DIR}/Util/Timer.h"
    "${POCO_DIR}/Util/TimerTaskAdapter.h"
    "${POCO_DIR}/Timestamp.h"
    "${POCO_DIR}/Logger.h"
    "${POCO_DIR}/DateTimeParser.h"
    "${POCO_DIR}/DateTimeFormat.h"
    "${WEBSOCKETPP_DIR}/client.hpp"
    "${WEBSOCKETPP_DIR}/config/asio_client.hpp"
    "include/json.hpp"
    "include/util.h"
    "include/bitmexhttp.h"
    "include/bitmexws.h"
    "include/delta_parser.h"
    "include/market_filtration.h"
    "include/fair_value.h"
)

# Add an executable for the file app/main.cpp.
add_executable(main app/main.cpp)   # Name of exec. and location of file.
target_link_libraries(main PRIVATE ${LIBRARY_NAME})  # Link the executable to library
target_set_warnings(main ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings
target_enable_lto(main optimized)  # enable link-time-optimization if available for non-debug configurations

# Project properties
set_target_properties(
    ${LIBRARY_NAME} main 
      PROPERTIES 
        CXX_STANDARD 17 
        CXX_STANDARD_REQUIRED YES 
        CXX_EXTENSIONS NO
)

# Set up tests (see tests/CMakeLists.txt).
add_subdirectory(tests)
