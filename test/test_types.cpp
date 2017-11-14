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
#include "types.ipp"

#include "statement.hpp"

#include "fixture.hpp"

#include <boost/fusion/include/std_tuple.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( number )

BOOST_AUTO_TEST_CASE( canConstructSmallNumber )
{
    const boost::multiprecision::cpp_int smallNumber{ 123 };

    const rodbc::Number< 5 > number{ smallNumber };
    BOOST_CHECK_EQUAL( smallNumber, number.value() );
}

BOOST_AUTO_TEST_CASE( canConstructLargeNumber )
{
    const boost::multiprecision::cpp_int largeNumber{ "12345678901234567890123456789012345678901234567890" };

    const rodbc::Number< 100 > number{ largeNumber };
    BOOST_CHECK_EQUAL( largeNumber, number.value() );
}

BOOST_AUTO_TEST_CASE( canDetectThatNumberIsTooLarge )
{
    const boost::multiprecision::cpp_int number{ "1234567890" };
    BOOST_CHECK_THROW( rodbc::Number< 5 >{ number }, std::range_error );
}

BOOST_AUTO_TEST_CASE( canConstructInt64 )
{
    const std::int64_t integer{ -23 };

    const rodbc::Number< 5 > number{ integer };
    BOOST_CHECK_EQUAL( integer, number.toInt64() );
}

BOOST_AUTO_TEST_CASE( canConstructUInt64 )
{
    const std::uint64_t integer{ 42 };

    const rodbc::Number< 5 > number{ integer };
    BOOST_CHECK_EQUAL( integer, number.toUInt64() );
}

BOOST_FIXTURE_TEST_CASE( canInsertAndSelectNumber, Fixture )
{
    rodbc::CreateTable< std::tuple< rodbc::Number< 10 > >, 0 >{
        conn, "tbl", { "col" },
        rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE
    };

    const boost::multiprecision::cpp_int number{ "1234567890" };

    {
        rodbc::Statement stmt{ conn, "INSERT INTO tbl (col) VALUES (?)" };

        const rodbc::Number< 10 > col{ number };
        stmt.bindParam( col );

        BOOST_CHECK_NO_THROW( stmt.exec() );
    }

    {
        rodbc::Statement stmt{ conn, "SELECT col FROM tbl" };

        rodbc::Number< 10 > col;
        stmt.bindCol( col );

        BOOST_CHECK_NO_THROW( stmt.exec() );
        BOOST_CHECK( stmt.fetch() );

        BOOST_CHECK_EQUAL( number, col.value() );
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( timestamp )

constexpr auto epoch = rodbc::Timestamp{
    1970, 1, 1, 0, 0, 0, 0
};

BOOST_AUTO_TEST_CASE( canConvertTheEpoch )
{
    BOOST_CHECK( epoch == rodbc::fromTime_t( 0 ) );
    BOOST_CHECK_EQUAL( 0, toTime_t( epoch ) );
}

BOOST_AUTO_TEST_CASE( canFormatTheEpoch )
{
    BOOST_CHECK_EQUAL( "1970-01-01T00:00:00.000Z", toString( epoch ) );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( exception, Fixture )

BOOST_AUTO_TEST_CASE( canDetectConstraintViolation )
{
    rodbc::CreateTable< std::tuple< int >, 0 >{
        conn, "tbl", { "col" },
        rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE
    };

    rodbc::Statement stmt{ conn, "INSERT INTO tbl (col) VALUES (?)" };

    const int col = 1;
    stmt.bindParam( col );

    BOOST_CHECK_NO_THROW( stmt.exec() );
    BOOST_CHECK_EXCEPTION( stmt.exec(), rodbc::Exception, std::mem_fn( &rodbc::Exception::isConstraintViolation ) );
}

BOOST_AUTO_TEST_SUITE_END()
