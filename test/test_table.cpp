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
#include "table.ipp"

#include "types.ipp"

#include "fixture.hpp"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE( table, Fixture )

BOOST_AUTO_TEST_CASE( canCrudSingleRow )
{
    using Table = rodbc::Table< std::tuple< int, rodbc::String< 32 > >, 0 >;

    const char* const tblName{ "tbl" };
    const Table::ColumnNames colNames{ "pk", "col" };

    Table::Create{
        conn, tblName, colNames, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE
    };

    Table table{
        conn, tblName, colNames
    };

    BOOST_CHECK( !table.select( std::make_tuple( 0 ) ).is_initialized() );

    BOOST_CHECK_NO_THROW( table.insert( std::make_tuple( 0, rodbc::String< 32 >{ "foobar" } ) ) );

    BOOST_CHECK_EQUAL( std::string{ "foobar" }, std::get< 1 >( *table.select( std::make_tuple( 0 ) ) ).str() );

    BOOST_CHECK_NO_THROW( table.update( std::make_tuple( 0 ), std::make_tuple( 0, rodbc::String< 32 >( "barfoo" ) ) ) );

    BOOST_CHECK_EQUAL( std::string{ "barfoo" }, std::get< 1 >( *table.select( std::make_tuple( 0 ) ) ).str() );

    BOOST_CHECK_NO_THROW( table.erase( std::make_tuple( 0 ) ) );

    BOOST_CHECK( !table.select( std::make_tuple( 0 ) ).is_initialized() );
}

BOOST_AUTO_TEST_SUITE_END()
