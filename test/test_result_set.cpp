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
#include "create_table.ipp"
#include "result_set.ipp"
#include "typed_statement.ipp"

#include "fixture.hpp"

#include <boost/test/unit_test.hpp>

#include <numeric>

BOOST_FIXTURE_TEST_SUITE( resultSet, Fixture )

BOOST_AUTO_TEST_CASE( canBeUsedInRangedForLoopsAndWithStlAlgorithms )
{
    rodbc::CreateTable< std::tuple< int > >{
        conn, "tbl", { "col" },
        rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE
    };

    rodbc::TypedStatement< std::tuple< int >, std::tuple<> > insertStmt{
        conn, "INSERT INTO tbl (col) VALUES (?)"
    };

    for ( int index = 0; index < 128; ++index )
    {
        auto& stmt = insertStmt;
        auto& params = stmt.params();

        std::get< 0 >( params ) = index;

        stmt.exec();
    }

    rodbc::TypedStatement< std::tuple<>, std::tuple< int > > selectStmt{
        conn, "SELECT col FROM tbl"
    };

    BOOST_CHECK_NO_THROW( selectStmt.exec() );

    for ( const auto& row : selectStmt )
    {
        BOOST_CHECK_LE( 0, std::get< 0 >( row ) );
        BOOST_CHECK_GT( 128, std::get< 0 >( row ) );
    }

    BOOST_CHECK_NO_THROW( selectStmt.exec() );

    const auto sum = std::accumulate( std::begin( selectStmt ), std::end( selectStmt ), 0, []( const int sum, const std::tuple< int >& row )
    {
        return sum + std::get< 0 >( row );
    } );

    BOOST_CHECK_EQUAL( 128 * ( 128 - 1 ) / 2, sum );
}

BOOST_AUTO_TEST_SUITE_END()
