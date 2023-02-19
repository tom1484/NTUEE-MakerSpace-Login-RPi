cmake_minimum_required(VERSION 3.10)
project(clock)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(LIBRARIES pthread)

add_executable(clock ${SOURCE}/clock.cpp)
target_include_directories(clock PUBLIC ${INCLUDE_DIRS})
target_link_libraries(clock ${LIBRARIES})