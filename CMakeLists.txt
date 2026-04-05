cmake_minimum_required(VERSION 3.25)

project(OrderBook_Proj)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(MAIN_EXEC_FILENAME main_exe)
set(TEST_EXEC_FILENAME test_exe)
set(GUI_EXEC_FILENAME  gui_exe)

add_executable(${MAIN_EXEC_FILENAME}
                   "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/src/orderbook.cpp"
)

target_include_directories(${MAIN_EXEC_FILENAME}
                               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Find the Google Test package
find_package(GTest REQUIRED)

# Test executable
add_executable(${TEST_EXEC_FILENAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_order_book.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/orderbook.cpp"
)

# Link Google Test libraries
target_link_libraries(${TEST_EXEC_FILENAME} PRIVATE GTest::GTest GTest::Main)

target_include_directories(${TEST_EXEC_FILENAME}
                               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                       "${GTEST_INCLUDE_DIRS}")

add_subdirectory(gui)
