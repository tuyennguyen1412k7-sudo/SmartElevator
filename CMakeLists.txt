cmake_minimum_required(VERSION 3.16)
project(SmartElevator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(elevator_core STATIC
  src/value_types.cpp
  src/building.cpp
  src/text_utils.cpp
  src/event.cpp
  src/request.cpp
  src/config.cpp
  src/elevator_state.cpp
  src/elevator.cpp
  src/dispatch_strategy.cpp
  src/controller.cpp
  src/persistence.cpp
  src/simulator.cpp)
target_include_directories(elevator_core PUBLIC include)
if(MSVC)
  target_compile_options(elevator_core PUBLIC /W4 /utf-8)
else()
  target_compile_options(elevator_core PUBLIC -Wall -Wextra -Wpedantic)
endif()

add_executable(elevator_sim app/main.cpp)
target_link_libraries(elevator_sim PRIVATE elevator_core)

add_executable(elevator_tests
  tests/test_main.cpp
  tests/test_utils.cpp
  tests/test_dispatch.cpp
  tests/test_elevator.cpp
  tests/test_persistence.cpp)
target_link_libraries(elevator_tests PRIVATE elevator_core)

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

enable_testing()
add_test(NAME elevator_tests COMMAND elevator_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
