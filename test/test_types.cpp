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
#include "types.hpp"

#include "statement.hpp"

#include "fixture.hpp"

#include <boost/fusion/include/std_tuple.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( timestamp )

constexpr auto epoch = rodbc::Timestamp{
    1970, 1, 1, 0, 0, 0, 0
};

BOOST_AUTO_TEST_CASE( canConvertTheEpoch )
{
    BOOST_CHECK( epoch == rodbc::from_time_t( 0 ) );
    BOOST_CHECK_EQUAL( 0, to_time_t( epoch ) );
}

BOOST_AUTO_TEST_CASE( canFormatTheEpoch )
{
    BOOST_CHECK_EQUAL( "1970-01-01T00:00:00.000Z", to_string( epoch ) );
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
