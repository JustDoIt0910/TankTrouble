cmake_minimum_required(VERSION 3.8)
project(TankTrouble)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})
link_libraries(${GTKMM_LIBRARIES})

add_executable(TankTrouble main.cc tank.cc util/cord.cc field.cc controller.cc bullet.cc util/math.cc)
