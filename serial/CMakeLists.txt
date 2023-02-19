cmake_minimum_required(VERSION 3.10)
project(serial)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(LIBRARIES wiringPi)

add_library(serial STATIC ${SOURCE}/serial.cpp)
target_include_directories(serial PUBLIC ${INCLUDE})
target_link_libraries(serial ${LIBRARIES})

target_include_directories(serial PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(serial ${CURL_LIBRARIES})

target_include_directories(serial PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(serial ${JSONCPP_LIBRARIES})