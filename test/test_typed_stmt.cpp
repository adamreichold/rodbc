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
#include "typed_statement.ipp"

#include "fixture.hpp"

#include <boost/test/unit_test.hpp>

namespace
{

void insertIndices( rodbc::Statement& stmt, int size )
{
    int param;
    stmt.rebindParams();
    stmt.bindParam( param );

    for ( int index = 0; index < size; ++index )
    {
        param = index;

        BOOST_CHECK_NO_THROW( stmt.exec() );
    }
}

void insertIndices( rodbc::TypedStatement< std::vector< std::tuple< int > >, std::tuple<> >& stmt, int size )
{
    auto& params = stmt.params();

    params.resize( size );

    for ( int index = 0; index < size; ++index )
    {
        params[ index ] = index;
    }

    BOOST_CHECK_NO_THROW( stmt.exec() );
}

void selectIndices( rodbc::Statement& stmt, int size )
{
    int col;
    stmt.rebindCols();
    stmt.bindCol( col );

    BOOST_CHECK_NO_THROW( stmt.exec() );

    for ( int index = 0; index < size; ++index )
    {
        BOOST_CHECK( stmt.fetch() );
        BOOST_CHECK_EQUAL( index, col );
    }

    BOOST_CHECK( !stmt.fetch() );
}

void selectIndices( rodbc::TypedStatement< std::tuple<>, std::vector< std::tuple< int > > >& stmt, int size )
{
    BOOST_CHECK_NO_THROW( stmt.exec() );

    for( int index = 0; index < size; )
    {
        BOOST_CHECK( stmt.fetch() );

        for ( const auto& cols : stmt.cols() )
        {
            BOOST_CHECK_EQUAL( index++, std::get< 0 >( cols ) );
        }
    }

    BOOST_CHECK( !stmt.fetch() );
}

}

BOOST_FIXTURE_TEST_SUITE( typedStmt, Fixture )

BOOST_AUTO_TEST_CASE( canRebindParameterSet )
{
    CreateSimpleTable< int >{ conn };

    rodbc::TypedStatement< std::vector< std::tuple< int > >, std::tuple<> > insertStmt{
        conn, "INSERT INTO tbl (col) VALUES (?)"
    };

    rodbc::Statement selectStmt{
        conn, "SELECT col FROM tbl ORDER BY col"
    };

    rodbc::Statement deleteStmt{
        conn, "DELETE FROM tbl"
    };

    insertStmt.params().reserve( 128 );
    auto* const data = insertStmt.params().data();

    insertIndices( insertStmt, 64 );
    BOOST_CHECK_EQUAL( data, insertStmt.params().data() );

    selectIndices( selectStmt, 64 );
    BOOST_CHECK_NO_THROW( deleteStmt.exec() );

    insertIndices( insertStmt, 128 );
    BOOST_CHECK_EQUAL( data, insertStmt.params().data() );

    selectIndices( selectStmt, 128 );
    BOOST_CHECK_NO_THROW( deleteStmt.exec() );

    insertIndices( insertStmt, 256 );
    BOOST_CHECK_NE( data, insertStmt.params().data() );

    selectIndices( selectStmt, 256 );
    BOOST_CHECK_NO_THROW( deleteStmt.exec() );
}

BOOST_AUTO_TEST_CASE( canRebindRowSet )
{
    CreateSimpleTable< int >{ conn };

    rodbc::Statement insertStmt{
        conn, "INSERT INTO tbl (col) VALUES (?)"
    };

    insertIndices( insertStmt, 256 );

    rodbc::TypedStatement< std::tuple<>, std::vector< std::tuple< int > > > selectStmt{
        conn, "SELECT col FROM tbl ORDER BY col", 128
    };

    const auto* const data = selectStmt.cols().data();
    selectIndices( selectStmt, 256 );

    selectStmt.setFetchSize( 64 );

    BOOST_CHECK_EQUAL( data, selectStmt.cols().data() );
    selectIndices( selectStmt, 256 );

    selectStmt.setFetchSize( 256 );

    BOOST_CHECK_NE( data, selectStmt.cols().data() );
    selectIndices( selectStmt, 256 );
}

BOOST_AUTO_TEST_SUITE_END()
