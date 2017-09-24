#include "connection_pool.ipp"

#include "create_table.ipp"
#include "typed_statement.ipp"

#include <boost/test/unit_test.hpp>

struct Statements
{
    rodbc::CreateTable< std::tuple< int > > createTbl;
    rodbc::TypedStatement< std::tuple<>, std::tuple< int > > selectStmt;

    Statements( rodbc::Connection& conn )
    : createTbl{ conn, "tbl", { "col" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , selectStmt{ conn, "SELECT col FROM tbl" }
    {
    }
};

BOOST_AUTO_TEST_SUITE( connPool )

BOOST_AUTO_TEST_SUITE( threadLocalConnPool )

BOOST_AUTO_TEST_CASE( canConnect )
{
    using Pool = rodbc::ThreadLocalConnectionPool< Statements >;

    Pool pool{ RODBC_TEST_CONN_STR };
    Pool::Lease lease{ pool };

    lease( []( rodbc::Connection& conn, Statements& stmts )
    {
        rodbc::Transaction trans{ conn };

        auto& stmt = stmts.selectStmt;
        BOOST_CHECK_NO_THROW( stmt.exec() );
        BOOST_CHECK( !stmt.fetch() );

        trans.commit();
    } );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( fixedSizeConnPool )

BOOST_AUTO_TEST_CASE( canConnect )
{
    using Pool = rodbc::FixedSizeConnectionPool< Statements >;

    Pool pool{ RODBC_TEST_CONN_STR, 1ul };
    Pool::Lease lease{ pool };

    lease( []( rodbc::Connection& conn, Statements& stmts )
    {
        rodbc::Transaction trans{ conn };

        auto& stmt = stmts.selectStmt;
        BOOST_CHECK_NO_THROW( stmt.exec() );
        BOOST_CHECK( !stmt.fetch() );

        trans.commit();
    } );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
