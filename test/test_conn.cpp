#include "connection.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( canConnect )
{
    rodbc::Connection conn{ RODBC_TEST_CONN_STR };
}

BOOST_AUTO_TEST_CASE( isNotDead )
{
    rodbc::Connection conn{ RODBC_TEST_CONN_STR };

    BOOST_CHECK( !conn.isDead() );
}

BOOST_AUTO_TEST_CASE( canStartTransaction )
{
    rodbc::Connection conn{ RODBC_TEST_CONN_STR };
    rodbc::Transaction trans{ conn };
}

BOOST_AUTO_TEST_CASE( canCommitTransaction )
{
    rodbc::Connection conn{ RODBC_TEST_CONN_STR };
    rodbc::Transaction trans{ conn };

    trans.commit();
}
