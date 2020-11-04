cmake_minimum_required(VERSION 3.3)

project(spi_rfid)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (BOOST_DIR)
  set(BOOST_ROOT ${BOOST_DIR})
endif (BOOST_DIR)

include(GNUInstallDirs)

find_package( Threads REQUIRED)
find_package( Boost 1.70.0 REQUIRED COMPONENTS system)

message("server build")

add_executable( read_rfid
    read_rfid.cpp
    spi.cpp spi.h
    PCD_Command.h PICC_Command.h
    Register.h
    rfid.cpp rfid.h
    Debounce.cpp Debounce.h
    repeattimer.h
    table.h table.cpp
    stickentry.h stickentry.cpp
    rfidinterface.h rfidinterface.cpp
)

set_target_properties(read_rfid PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

target_include_directories(
        read_rfid
        PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries( read_rfid
        Threads::Threads
        ${Boost_LIBRARIES}
        snc_client
)

# ----------------------------------------------------------------

add_executable(${PROJECT_NAME}
    spi_rfid.cpp
    spi.cpp spi.h
    PCD_Command.h PICC_Command.h
    Register.h
    rfid.cpp rfid.h
    Debounce.cpp Debounce.h
    repeattimer.h
    table.h table.cpp
    stickentry.h stickentry.cpp
    rfidinterface.h rfidinterface.cpp
    )

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

# ----------------------------------------------------------------

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

# -----------------------------------------

install(
    TARGETS ${PROJECT_NAME} requester read_rfid
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    )


# ------------------------------------------

set(LOCAL_CONFIG_DIR ${CMAKE_SOURCE_DIR}/conf)
set(LOCAL_CONFIG_FILE ${LOCAL_CONFIG_DIR}/keyTable.json)

set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/audioserver/stick)
set(INSTALL_CONFIG_FILE ${INSTALL_CONFIG_DIR}/keyTable.json)

install ( DIRECTORY DESTINATION ${INSTALL_CONFIG_DIR} DIRECTORY_PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
           GROUP_READ
           WORLD_READ  )

install(
          FILES ${LOCAL_CONFIG_FILE} DESTINATION ${INSTALL_CONFIG_DIR}
)

# ------------------------------------------

