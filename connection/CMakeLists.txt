cmake_minimum_required(VERSION 3.10)
project(connection)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(LIBRARIES "")

add_library(connection STATIC ${SOURCE}/connection.cpp)
target_include_directories(connection PUBLIC ${INCLUDE})
target_link_libraries(connection ${LIBRARIES})

target_include_directories(connection PUBLIC ${CURLPP_INCLUDE_DIRS})
target_link_libraries(connection ${CURLPP_LIBRARIES})

target_include_directories(connection PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(connection ${JSONCPP_LIBRARIES})