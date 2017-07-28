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
