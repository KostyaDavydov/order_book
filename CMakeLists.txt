cmake_minimum_required(VERSION 3.25)

project(order_book_proj)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(MAIN_EXEC_FILENAME order_book)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
                  "${CMAKE_CURRENT_SOURCE_DIR}/src/orderbook.cpp")
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/orderbook.h")

add_executable(${MAIN_EXEC_FILENAME} ${SOURCES} ${HEADERS})

target_include_directories(${MAIN_EXEC_FILENAME}
                               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
