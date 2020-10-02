cmake_minimum_required(VERSION 3.3)

project(spi_rfid)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (BOOST_DIR)
  set(BOOST_ROOT ${BOOST_DIR})
endif (BOOST_DIR)

find_package( Threads REQUIRED)
find_package( Boost 1.70.0 REQUIRED COMPONENTS system)

message("server build")

add_executable(${PROJECT_NAME} testSequence.cpp spi.cpp spi.h PCD_Command.h PICC_Command.h Register.h rfid.cpp rfid.h Debounce.cpp Debounce.h repeattimer.h)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
        Threads::Threads
        ${Boost_LIBRARIES}
        snc_client
)

add_executable(requester requester.cpp)
set_target_properties(requester PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_include_directories(
        requester
        PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(requester
        Threads::Threads
        ${Boost_LIBRARIES}
        snc_client
)
