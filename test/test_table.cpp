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
    rodbc::Table< std::tuple< int, rodbc::String< 32 > >, 0 > table{
        conn, "tbl", { "pk", "col" }
    };

    table.create( rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE );

    BOOST_CHECK( !table.select( 0 ).is_initialized() );

    BOOST_CHECK_NO_THROW( table.insert( std::make_tuple( 0, rodbc::String< 32 >{ "foobar" } ) ) );

    BOOST_CHECK_EQUAL( std::string{ "foobar" }, std::get< 1 >( *table.select( 0 ) ).str() );

    BOOST_CHECK_NO_THROW( table.update( std::make_tuple( 0, rodbc::String< 32 >( "barfoo" ) ) ) );

    BOOST_CHECK_EQUAL( std::string{ "barfoo" }, std::get< 1 >( *table.select( 0 ) ).str() );

    BOOST_CHECK_NO_THROW( table.delete_( 0 ) );

    BOOST_CHECK( !table.select( 0 ).is_initialized() );
}

BOOST_AUTO_TEST_CASE( canSelectByArbitraryKey )
{
    rodbc::Table< std::tuple< int, rodbc::String< 32 > >, 0 > table{
        conn, "tbl", { "pk", "col" }
    };

    table.create( rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE );

    for ( int index = 0; index < 128; ++index )
    {
        table.insert( std::make_tuple( index, rodbc::String< 32 >{ std::to_string( index % 2 ) }) );
    }

    const auto evenRows = table.selectBy< 1 >( rodbc::String< 32 >{ "0" } );

    BOOST_CHECK_EQUAL( 64, evenRows.size() );

    for ( const auto& row : evenRows )
    {
        BOOST_CHECK( std::get< 0 >( row ) % 2 == 0 );
    }

    const auto oddRows = table.selectBy< 1 >( rodbc::String< 32 >{ "1" } );

    BOOST_CHECK_EQUAL( 64, oddRows.size() );

    for ( const auto& row : oddRows )
    {
        BOOST_CHECK( std::get< 0 >( row ) % 2 == 1 );
    }
}


BOOST_AUTO_TEST_CASE( canUpdateArbitraryValuesByArbitraryKey )
{
    rodbc::Table< std::tuple< int, rodbc::String< 32 > >, 0 > table{
        conn, "tbl", { "pk", "col" }
    };

    table.create( rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE );

    for ( int index = 0; index < 128; ++index )
    {
        table.insert( std::make_tuple( index, rodbc::String< 32 >{ std::to_string( index ) }) );
    }

    const auto row = std::make_tuple( -127, rodbc::String< 32 >{ "127" } );

    table.updateAtBy( row, rodbc::IndexSequence< 0 >{}, rodbc::IndexSequence< 1 >{} );

    const auto rows = table.selectAll();

    BOOST_CHECK_EQUAL( 128, rows.size() );

    for ( const auto& row : rows )
    {
        const auto pk = std::get< 0 >( row );
        const auto col = std::get< 1 >( row ).str();

        if ( pk < 0 )
        {
            BOOST_CHECK_EQUAL( -127, pk );
            BOOST_CHECK_EQUAL( std::string{ "127" }, col );
        }
        else
        {
            BOOST_CHECK_EQUAL( std::to_string( pk ), col );
        }
    }
}

BOOST_AUTO_TEST_CASE( canUseSubtypeForExtension )
{
    using Columns = std::tuple< int, rodbc::String< 32 > >;

    class Table : public rodbc::Table< Columns, 0 >
    {
    public:
        Table( rodbc::Connection& conn )
        : rodbc::Table< Columns, 0 >{ conn, "tbl", { "pk", "col" } }
        {
        }

        std::vector< Columns > selectByPkLessThan( const int pk ) const
        {
            std::vector< Columns > rows;

            if ( !selectByPkLessThan_ )
            {
                selectByPkLessThan_.emplace( conn_, "SELECT pk, col FROM tbl WHERE pk < ? ORDER BY pk" );
            }

            selectByPkLessThan_->params() = std::forward_as_tuple( pk );

            selectByPkLessThan_->exec();

            while ( selectByPkLessThan_->fetch() )
            {
                rows.push_back( selectByPkLessThan_->cols() );
            }

            return rows;
        }

    private:
        mutable boost::optional< rodbc::TypedStatement< std::tuple< int >, Columns > > selectByPkLessThan_;
    };

    Table table{ conn };

    table.create( rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE );

    for ( int index = 0; index < 128; ++index )
    {
        table.insert( std::make_tuple( index, rodbc::String< 32 >{ std::to_string( index ) }) );
    }

    const auto rows = table.selectByPkLessThan( 64 );

    BOOST_CHECK_EQUAL( 64, rows.size() );

    for ( int index = 0; index < 64; ++index )
    {
        const auto& row = rows[ index ];

        BOOST_CHECK_EQUAL( index, std::get< 0 >( row ) );
        BOOST_CHECK_EQUAL( std::to_string( index ), std::get< 1 >( row ).str() );
    }
}

BOOST_AUTO_TEST_SUITE_END()
