#pragma once

#include "connection.hpp"
#include "statement.hpp"

struct Fixture
{
    rodbc::Connection conn{ RODBC_TEST_CONN_STR };
};

template< typename Value >
inline void resizeRowSet( std::vector< Value >& values, const std::size_t size )
{
    values.resize( size );
    values.shrink_to_fit();

    assert( values.size() == values.capacity() );
}

inline void createTable( rodbc::Connection& conn, const std::string& name, const std::string& definition )
{
    const auto dropStmt = "DROP TABLE IF EXISTS " + name + ";";
    const auto createStmt = "CREATE TABLE " + name + " (" + definition + ");";

    rodbc::Statement{ conn, dropStmt.c_str() }.exec();
    rodbc::Statement{ conn, createStmt.c_str() }.exec();
}
