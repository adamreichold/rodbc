#pragma once

#include "connection.hpp"
#include "statement.hpp"

struct Fixture
{
    rodbc::Environment env;
    rodbc::Connection conn{ env, RODBC_TEST_CONN_STR };
};
