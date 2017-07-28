add_definitions( "'-DRODBC_TEST_CONN_STR=\"Driver=SQLITE3;Database=:memory:;\"'" )

find_package( Boost 1.60 COMPONENTS unit_test_framework REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
add_definitions( -DBOOST_TEST_DYN_LINK )

function( add_test_executable test_name executable_name source )
    add_executable( ${executable_name} ${source} )
    target_compile_definitions( ${executable_name} PRIVATE "-DBOOST_TEST_MODULE=${test_name}" )
    target_link_libraries( ${executable_name} rodbc ${Boost_LIBRARIES} )
    add_test( ${test_name} ${executable_name} )
endfunction()

add_test_executable( types test_types test_types.cpp )
add_test_executable( connection test_conn test_conn.cpp )
add_test_executable( statement test_stmt test_stmt.cpp )
add_test_executable( typed_statement test_typed_stmt test_typed_stmt.cpp )
add_test_executable( database test_db "db.cpp;test_db.cpp" )
