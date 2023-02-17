cmake_minimum_required(VERSION 3.8)
project(TankTrouble)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(TANK_TROUBLE_SRC main.cc
        Tank.cc
        Shell.cc
        Block.cc
        util/Cord.cc
        util/Math.cc
        Window.cc
        View.cc
        Controller.cc
        event/ControlEvent.cc)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

set(EV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ev/ev)
set(EV_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/ev/ev/reactor)
add_subdirectory(ev)

include_directories(${GTKMM_INCLUDE_DIRS} ${EV_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS} ${EV_LIBRARY_DIRS})
link_libraries(${GTKMM_LIBRARIES} reactor pthread)

add_executable(TankTrouble ${TANK_TROUBLE_SRC})
