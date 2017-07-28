cmake_minimum_required( VERSION 3.1 )

set( CMAKE_CURRENT_SOURCE_DIR ON )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )

if( NOT CMAKE_BUILD_TYPE MATCHES Debug )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto" )
    set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto" )
endif()

project( rodbc CXX )

add_library( rodbc SHARED src/types.cpp src/connection.cpp src/statement.cpp )
include_directories( include )

set_target_properties(
    rodbc PROPERTIES
    VERSION 0.1
    SOVERSION 0
)

target_link_libraries( rodbc odbc )

find_package( Boost 1.60 COMPONENTS thread REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
target_link_libraries( rodbc ${Boost_LIBRARIES} )

install(
    TARGETS rodbc
    LIBRARY DESTINATION lib
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include/rodbc
    FILES_MATCHING PATTERN *.hpp|*.ipp
)

enable_testing()
add_subdirectory( test )
