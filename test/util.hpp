#pragma once

#include "connection.hpp"
#include "statement.hpp"

#include <boost/format.hpp>

struct Fixture
{
    rodbc::Environment env;
    rodbc::Connection conn{ env, RODBC_TEST_CONN_STR };
};

inline void createTable( rodbc::Connection& conn, const char* name, const char* definition )
{
    rodbc::Statement{ conn, str( boost::format( "DROP TABLE IF EXISTS %s;" ) % name ).c_str() }.exec();
    rodbc::Statement{ conn, str( boost::format( "CREATE TEMPORARY TABLE %s (%s);" ) % name % definition ).c_str() }.exec();
}
