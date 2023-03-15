include_directories(${CMAKE_SOURCE_DIR}/reactor ${CMAKE_SOURCE_DIR}/net)
link_directories(${CMAKE_BINARY_DIR}/ev/reactor ${CMAKE_BINARY_DIR}/ev/net)
link_libraries(reactor net pthread)

add_executable(loop loop.cc)
add_executable(timer timer.cc)
add_executable(connect connect.cc)
add_executable(threadpool threadpool.cc ../utils/ThreadPool.cc)