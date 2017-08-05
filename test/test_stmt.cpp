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
#include "statement.hpp"

#include "create_table.hpp"

#include "fixture.hpp"

#include <boost/fusion/include/std_tuple.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

namespace
{

using SignedTypes = boost::mpl::list< std::int8_t, std::int16_t, std::int32_t, std::int64_t >;
using UnsignedTypes = boost::mpl::list< std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t >;
using IntegerTypes = boost::mpl::joint_view< SignedTypes, UnsignedTypes >;
using FloatingPointTypes = boost::mpl::list< float, double >;
using NumericTypes = boost::mpl::joint_view< IntegerTypes, FloatingPointTypes >;

template< typename Type >
bool createSingleColumnTable( rodbc::Connection& conn )
{
    if ( conn.dbms() == rodbc::DBMS::PostgreSQL && ( sizeof ( Type ) == 1 || std::is_unsigned< Type >::value ) )
    {
        BOOST_TEST_MESSAGE( "PostgreSQL does not support tiny or unsigned integer types." );

        return false;
    }

    rodbc::CreateTable< std::tuple< Type > >{
        conn, "tbl", { "col" },
        rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE
    };

    return true;
}

template< typename Type >
void insertAndSelectSingleValue( rodbc::Connection& conn, const Type& test, const Type& sentinel )
{
    {
        rodbc::Statement insert{ conn, "INSERT INTO tbl (col) VALUES (?)" };

        const Type value{ test };
        insert.bindParam( value );

        BOOST_CHECK_NO_THROW( insert.exec() );
    }

    {
        rodbc::Statement select{ conn, "SELECT col FROM tbl" };

        Type value{ sentinel };
        select.bindCol( value );

        BOOST_CHECK_NO_THROW( select.exec() );
        BOOST_CHECK( select.fetch() );
        BOOST_CHECK_EQUAL( test, value );
    }
}

template< typename Type >
void insertAndSelectSingleNullValue( rodbc::Connection& conn, const Type& sentinel )
{
    {
        rodbc::Statement insert{ conn, "INSERT INTO tbl (col) VALUES (?)" };

        const rodbc::Nullable< Type > value;
        insert.bindParam( value );

        BOOST_CHECK_NO_THROW( insert.exec() );
    }

    {
        rodbc::Statement select{ conn, "SELECT col FROM tbl" };

        rodbc::Nullable< Type > value{ sentinel };
        select.bindCol( value );

        BOOST_CHECK_NO_THROW( select.exec() );
        BOOST_CHECK( select.fetch() );
        BOOST_CHECK( value.isNull() );
    }
}

}

BOOST_FIXTURE_TEST_SUITE( stmt, Fixture )

BOOST_AUTO_TEST_CASE_TEMPLATE( canInsertAndSelectZero, Type, NumericTypes )
{
    if ( !createSingleColumnTable< Type >( conn ) )
    {
        return;
    }

    insertAndSelectSingleValue( conn, Type( 0 ), Type( 1 ) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE( canInsertAndSelectOne, Type, NumericTypes )
{
    if ( !createSingleColumnTable< Type >( conn ) )
    {
        return;
    }

    insertAndSelectSingleValue( conn, Type( 1 ), Type( 0 ) );
}

BOOST_AUTO_TEST_CASE_TEMPLATE( canInsertAndSelectMinimum, Type, IntegerTypes )
{
    if ( !createSingleColumnTable< Type >( conn ) )
    {
        return;
    }

    insertAndSelectSingleValue( conn, std::numeric_limits< Type >::min(), std::numeric_limits< Type >::max() );
}

BOOST_AUTO_TEST_CASE_TEMPLATE( canInsertAndSelectMaximum, Type, IntegerTypes )
{
    if ( !createSingleColumnTable< Type >( conn ) )
    {
        return;
    }

    insertAndSelectSingleValue( conn, std::numeric_limits< Type >::max(), std::numeric_limits< Type >::min() );
}

BOOST_AUTO_TEST_CASE_TEMPLATE( canInsertAndSelectNull, Type, NumericTypes )
{
    if ( !createSingleColumnTable< Type >( conn ) )
    {
        return;
    }

    insertAndSelectSingleNullValue( conn, Type( 0 ) );
}

BOOST_AUTO_TEST_CASE( canInsertAndSelectString )
{
    if ( !createSingleColumnTable< rodbc::String< 32 > >( conn ) )
    {
        return;
    }

    insertAndSelectSingleValue( conn, rodbc::String< 32 >{ "FOOBAR" }, rodbc::String< 32 > { "" } );
}

BOOST_AUTO_TEST_CASE( canInsertAndSelectEmptyString )
{
    if ( !createSingleColumnTable< rodbc::String< 32 > >( conn ) )
    {
        return;
    }

    insertAndSelectSingleValue( conn, rodbc::String< 32 >{ "" }, rodbc::String< 32 > { "FOOBAR" } );
}

BOOST_AUTO_TEST_SUITE_END()
