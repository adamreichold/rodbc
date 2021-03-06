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
#include "staged_statement.ipp"

#include "fixture.hpp"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE( stagedStmt, Fixture )

BOOST_AUTO_TEST_CASE( canSelectByJoiningStagingTable )
{
    rodbc::Table< std::tuple< int, int, int >, 0, 1 > tbl{
        conn, "tbl", { "a", "b", "c" }
    };

    tbl.create( rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE );

    for ( int index = 0; index < 128; ++index )
    {
        tbl.insert( std::make_tuple( index, index * index, index * index * index ) );
    }

    rodbc::StagedStatement< std::tuple< int, int >, std::tuple<>, std::tuple< std::int32_t, int >, std::int16_t > selectStmt{
        conn, "stg_tbl", { "stg_idx", "x", "y" },
        "SELECT stg_idx, c FROM tbl, stg_tbl WHERE a = x AND b = y ORDER BY stg_idx"
    };

    selectStmt.resizeStagedParams( 32 );

    for ( int index = 0; index < 32; ++index )
    {
        auto& params = selectStmt.stagedParams( index );

        std::get< 0 >( params ) = 4 * index;
        std::get< 1 >( params ) = 4 * index * 4 * index;
    }

    BOOST_CHECK_NO_THROW( selectStmt.exec() );

    for ( int index = 0; index < 32; ++index )
    {
        auto& stmt = selectStmt;
        const auto& cols = stmt.cols();

        BOOST_CHECK( stmt.fetch() );

        BOOST_CHECK_EQUAL( index, std::get< 0 >( cols ) );
        BOOST_CHECK_EQUAL( 4 * index * 4 * index * 4 * index, std::get< 1 >( cols ) );
    }

    BOOST_CHECK( !selectStmt.fetch() );
}

BOOST_AUTO_TEST_SUITE_END()
