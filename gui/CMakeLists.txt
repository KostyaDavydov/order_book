set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt package
find_package(Qt5 COMPONENTS Core Widgets PrintSupport REQUIRED)

add_executable(${GUI_EXEC_FILENAME}
                   "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/src/orderbook_widget.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/libs/qcustomplot.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../src/orderbook.cpp"

                   "${CMAKE_CURRENT_SOURCE_DIR}/include/orderbook_widget.h"
)

target_link_libraries(${GUI_EXEC_FILENAME}
                          PRIVATE
                            Qt5::Core
                            Qt5::Widgets
                            Qt5::PrintSupport
)

target_include_directories(${GUI_EXEC_FILENAME}
                               PRIVATE
                                 "${CMAKE_CURRENT_SOURCE_DIR}/libs"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
