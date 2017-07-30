cmake_minimum_required( VERSION 2.8 )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall" )

if( NOT CMAKE_BUILD_TYPE MATCHES Debug )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto" )
    set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto" )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto" )
endif()

project( rodbc CXX )

find_package( Boost 1.60 COMPONENTS thread REQUIRED )
include_directories( include ${Boost_INCLUDE_DIR} )

add_library( rodbc SHARED src/types.cpp src/connection.cpp src/statement.cpp )
set_target_properties( rodbc PROPERTIES VERSION 0.1 SOVERSION 0 )
target_link_libraries( rodbc odbc ${Boost_LIBRARIES} )

install( TARGETS rodbc LIBRARY DESTINATION lib )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/include DESTINATION include/rodbc FILES_MATCHING PATTERN *.hpp|*.ipp )

set( CPACK_PACKAGE_VERSION 0.1 )
set( CPACK_RPM_PACKAGE_LICENSE LGPLv3+ )
set( CPACK_RPM_PACKAGE_URL https://github.com/adamreichold/rodbc )
include( CPack )

enable_testing()
add_subdirectory( test )
