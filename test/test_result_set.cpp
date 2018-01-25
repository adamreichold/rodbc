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
#include "result_set.ipp"

#include "typed_statement.ipp"

#include "fixture.hpp"

#include <boost/test/unit_test.hpp>

#include <numeric>

namespace
{

void createTableAndInsertValues( rodbc::Connection& conn )
{
    CreateSimpleTable< int >{ conn };

    rodbc::TypedStatement< std::tuple< int >, std::tuple<> > stmt{
        conn, "INSERT INTO tbl (col) VALUES (?)"
    };

    for ( int index = 0; index < 128; ++index )
    {
        std::get< 0 >( stmt.params() ) = index;

        stmt.exec();
    }
}

}

BOOST_FIXTURE_TEST_SUITE( resultSet, Fixture )

BOOST_AUTO_TEST_CASE( canBeUsedInRangedForLoopsAndWithStlAlgorithms )
{
    createTableAndInsertValues( conn );

    rodbc::TypedStatement< std::tuple<>, std::tuple< int > > stmt{
        conn, "SELECT col FROM tbl"
    };

    for ( const auto& row : rodbc::ResultSet< std::tuple< int > >{ stmt } )
    {
        BOOST_CHECK_LE( 0, std::get< 0 >( row ) );
        BOOST_CHECK_GT( 128, std::get< 0 >( row ) );
    }

    rodbc::ResultSet< std::tuple< int > > results{ stmt };

    const auto sum = std::accumulate( results.begin(), results.end(), 0, []( const int sum, const std::tuple< int >& row )
    {
        return sum + std::get< 0 >( row );
    } );

    BOOST_CHECK_EQUAL( 128 * ( 128 - 1 ) / 2, sum );
}

BOOST_AUTO_TEST_CASE( canIterateThroughRowSets )
{
    createTableAndInsertValues( conn );

    rodbc::TypedStatement< std::tuple<>, std::vector< std::tuple< int > > > stmt{
        conn, "SELECT col FROM tbl", 3
    };

    rodbc::ResultSet< std::vector< std::tuple< int > > > results{ stmt };

    const auto sum = std::accumulate( results.begin(), results.end(), 0, []( const int sum, const std::tuple< int >& row )
    {
        return sum + std::get< 0 >( row );
    } );

    BOOST_CHECK_EQUAL( 128 * ( 128 - 1 ) / 2, sum );
}

BOOST_AUTO_TEST_SUITE_END()
