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

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Timestamp )

BOOST_AUTO_TEST_CASE( canFormatTheEpoch )
{
    const auto epoch = rodbc::Timestamp{
        1970, 1, 1, 0, 0, 0, 0
    }.str();

    BOOST_CHECK_EQUAL( "1970-01-01T00:00:00.000Z", epoch );
}

BOOST_AUTO_TEST_SUITE_END()
