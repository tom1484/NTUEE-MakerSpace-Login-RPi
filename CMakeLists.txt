cmake_minimum_required(VERSION 3.10)
project(access)

find_package(CURL)
find_package(jsoncpp)
find_package(OpenCV)

set(EXECUTABLES frame clock access)
set(INSTALL /usr/local/bin)

add_subdirectory(connection)
add_subdirectory(serial)
add_subdirectory(frame)
add_subdirectory(clock)
add_subdirectory(access)

install(TARGETS ${EXECUTABLES} RUNTIME DESTINATION ${INSTALL})