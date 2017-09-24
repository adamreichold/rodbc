/*

Copyright 2017 Adam Reichold

This file is part of rodbc.

rodbc is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rodbc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with rodbc.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "db.hpp"

#include <boost/test/unit_test.hpp>

struct FoobarFixture
{
    foobar::Database db{ RODBC_TEST_CONN_STR };
};

BOOST_FIXTURE_TEST_SUITE( foobarDb, FoobarFixture )

BOOST_AUTO_TEST_CASE( canCreateTables )
{
    foobar::Database::Transaction trans{ db };
    BOOST_CHECK_NO_THROW( trans.commit() );
}

BOOST_AUTO_TEST_CASE( canInsertAndSelectFoos )
{
    foobar::Database::Transaction trans{ db };

    {
        foobar::Database::InsertFoo stmt{ trans };

        for ( int i = 0; i < 256; ++i )
        {
            stmt.foo.x = i;
            stmt.foo.y = i * i;
            stmt.foo.z = i * i * i;

            BOOST_CHECK_NO_THROW( stmt.exec() );
        }
    }

    {
        foobar::Database::SelectAllFoo stmt{ trans };

        BOOST_CHECK_NO_THROW( stmt.exec() );

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
        foobar::Database::InsertBar stmt{ trans };

        stmt.bar.resize( 2048 );

        for ( int i = 0; i < 2048; ++i )
        {
            stmt.bar[ i ].a = 1.0f * i;
            stmt.bar[ i ].b = 2.0f * i;
            stmt.bar[ i ].c = 3.0f * i;
        }

        BOOST_CHECK_NO_THROW( stmt.exec() );
    }

    {
        foobar::Database::SelectBarByA stmt{ trans };

        stmt.a = 576.0f;

        BOOST_CHECK_NO_THROW( stmt.exec() );

        std::size_t remaining = 576;

        while ( stmt.fetch() )
        {
            remaining -= stmt.bar.size();

            for ( const auto& bar : stmt.bar )
            {
                BOOST_CHECK_GE( bar.a, 0.0f );
                BOOST_CHECK_LT( bar.a, 4096.0f );
                BOOST_CHECK_CLOSE( bar.b - bar.a, bar.a, 0.1 );
                BOOST_CHECK_CLOSE( bar.c - bar.b, bar.a, 0.1 );
            }
        }

        BOOST_CHECK_EQUAL( 0, remaining );
    }
}

BOOST_AUTO_TEST_SUITE_END()

struct BarfooFixture
{
    std::unique_ptr< barfoo::Database > db{ new barfoo::DatabaseImpl{ RODBC_TEST_CONN_STR, 1 } };
};

BOOST_FIXTURE_TEST_SUITE( barfooDb, BarfooFixture )

BOOST_AUTO_TEST_CASE( canInsertAndSelectFoos )
{
    auto trans = db->startTransaction();

    {
        for ( int i = 0; i < 256; ++i )
        {
            db->insertFoo( *trans, { i, i * i, i * i * i } );
        }
    }

    {
        const auto foos = db->selectAllFoo( *trans );

        BOOST_CHECK_EQUAL( 256, foos.size() );

        for ( const auto& foo : foos )
        {
            BOOST_CHECK_GE( foo.x, 0 );
            BOOST_CHECK_LT( foo.x, 256 );
            BOOST_CHECK_EQUAL( foo.y, foo.x * foo.x );
            BOOST_CHECK_EQUAL( foo.z, foo.x * foo.x * foo.x );
        }
    }
}


BOOST_AUTO_TEST_CASE( canInsertAndSelectBars )
{
    auto trans = db->startTransaction();

    {
        std::vector< barfoo::Bar > bars;
        bars.resize( 2048 );

        for ( int i = 0; i < 2048; ++i )
        {
            bars[ i ].a = 1.0f * i;
            bars[ i ].b = 2.0f * i;
            bars[ i ].c = 3.0f * i;
        }

        db->insertBar( *trans, bars );
    }

    {
        const auto bars = db->selectBarByA( *trans, 512.0f );

        BOOST_CHECK_EQUAL( 512, bars.size() );

        for ( const auto& bar : bars )
        {
            BOOST_CHECK_GE( bar.a, 0.0f );
            BOOST_CHECK_LT( bar.a, 4096.0f );
            BOOST_CHECK_CLOSE( bar.b - bar.a, bar.a, 0.1 );
            BOOST_CHECK_CLOSE( bar.c - bar.b, bar.a, 0.1 );
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
