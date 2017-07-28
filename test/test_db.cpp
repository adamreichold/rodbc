#include "db.hpp"

#include <boost/test/unit_test.hpp>

struct TestDb
{
    foobar::Database db{ RODBC_TEST_CONN_STR };

    TestDb()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE( testDb, TestDb )

BOOST_AUTO_TEST_CASE( canCreateTables )
{
    foobar::Database::Transaction trans{ db };
    trans.commit();
}

BOOST_AUTO_TEST_CASE( canInsertAndSelectFoos )
{
    foobar::Database::Transaction trans{ db };

    {
        foobar::Database::InsertFoo stmt{ db };

        for ( int i = 0; i < 256; ++i )
        {
            stmt.foo.x = i;
            stmt.foo.y = i * i;
            stmt.foo.z = i * i * i;

            stmt.exec();
        }
    }

    {
        foobar::Database::SelectAllFoo stmt{ db };

        stmt.exec();

        for ( int i = 0; i < 256; ++i )
        {
            BOOST_CHECK( stmt.fetch() );

            BOOST_CHECK_GE( stmt.foo.x, 0 );
            BOOST_CHECK_LT( stmt.foo.x, 256 );
            BOOST_CHECK_EQUAL( stmt.foo.y, stmt.foo.x * stmt.foo.x );
            BOOST_CHECK_EQUAL( stmt.foo.z, stmt.foo.x * stmt.foo.x * stmt.foo.x );
        }

        BOOST_CHECK( !stmt.fetch() );
    }
}

BOOST_AUTO_TEST_CASE( canInsertAndSelectBars )
{
    foobar::Database::Transaction trans{ db };

    {
        foobar::Database::InsertBar stmt{ db };

        stmt.bar.resize( 2048 );

        for ( int i = 0; i < 2048; ++i )
        {
            stmt.bar[ i ].a = 1.0f * i;
            stmt.bar[ i ].b = 2.0f * i;
            stmt.bar[ i ].c = 3.0f * i;
        }

        stmt.exec();
    }

    {
        foobar::Database::SelectBarByA stmt{ db, 128 };

        stmt.a = 512.0f;

        stmt.exec();

        for ( int i = 0; i < 4; ++i )
        {
            BOOST_CHECK( stmt.fetch() );
            BOOST_CHECK( stmt.bar.size() == 128 );

            for ( const auto& bar : stmt.bar )
            {
                BOOST_CHECK_GE( bar.a, 0.0f );
                BOOST_CHECK_LT( bar.a, 4096.0f );
                BOOST_CHECK_CLOSE( bar.b - bar.a, bar.a, 0.1 );
                BOOST_CHECK_CLOSE( bar.c - bar.b, bar.a, 0.1 );
            }
        }

        BOOST_CHECK( !stmt.fetch() );
    }
}

BOOST_AUTO_TEST_SUITE_END()
