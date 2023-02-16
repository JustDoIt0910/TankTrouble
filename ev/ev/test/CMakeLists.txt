include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../reactor)
link_directories(${CMAKE_BINARY_DIR}/ev/reactor)
link_libraries(reactor)

add_executable(loop loop.cc)
add_executable(timer timer.cc)