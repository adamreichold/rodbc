set( CMAKE_CURRENT_SOURCE_DIR ON )

set( RODBC_TEST_CONN_STR "\"Driver=SQLITE3;Database=:memory:;\"" CACHE STRING "ODBC connection string used to execute unit tests." )

find_package( Boost 1.60 COMPONENTS unit_test_framework REQUIRED )
add_definitions( "'-DRODBC_TEST_CONN_STR=${RODBC_TEST_CONN_STR}'" )

function( add_test_executable test_name executable_name source )
    add_executable( ${executable_name} ${source} )
    target_compile_definitions( ${executable_name} PRIVATE BOOST_TEST_DYN_LINK BOOST_TEST_MODULE=${test_name} )
    target_link_libraries( ${executable_name} rodbc ${Boost_LIBRARIES} )
    add_test( ${test_name} ${executable_name} )
endfunction()

add_test_executable( types test_types test_types.cpp )
add_test_executable( connection test_conn test_conn.cpp )
add_test_executable( statement test_stmt test_stmt.cpp )
add_test_executable( typed_statement test_typed_stmt test_typed_stmt.cpp )
add_test_executable( staged_statement test_staged_stmt test_staged_stmt.cpp )
add_test_executable( result_set test_result_set test_result_set.cpp )
add_test_executable( database test_db "db.cpp;test_db.cpp" )
