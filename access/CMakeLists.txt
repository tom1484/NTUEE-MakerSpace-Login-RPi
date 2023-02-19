cmake_minimum_required(VERSION 3.10)
project(access)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(LIBRARIES pthread serial connection)

add_executable(access ${SOURCE}/access.cpp)
target_include_directories(access PUBLIC ${INCLUDE_DIRS})
target_link_libraries(access ${LIBRARIES})
