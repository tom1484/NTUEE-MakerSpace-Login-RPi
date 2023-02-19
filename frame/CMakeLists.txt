cmake_minimum_required(VERSION 3.10)
project(frame)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(LIBRARIES pthread)

add_executable(frame ${SOURCE}/frame.cpp)
target_include_directories(frame PUBLIC ${INCLUDE_DIRS})
target_link_libraries(frame ${LIBRARIES})

target_include_directories(frame PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(frame ${OpenCV_LIBRARIES})