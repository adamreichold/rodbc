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
#pragma once

#include "database.hpp"

#include <boost/fusion/include/define_struct.hpp>

#include <vector>

BOOST_FUSION_DEFINE_STRUCT(
    ( foobar ), Foo,
    ( int, x )
    ( int, y )
    ( int, z )
);

BOOST_FUSION_DEFINE_STRUCT(
    ( foobar ), Bar,
    ( float, a )
    ( float, b )
    ( float, c )
);

namespace foobar
{

struct Statements;

class Database : public rodbc::Database< Statements >
{
public:
    Database( const char* const connStr );

    struct Transaction : BoundTransaction
    {
        Transaction( Database& database );

        void commit();
    };

    struct InsertFoo : BoundStatement
    {
        Foo& foo;

        InsertFoo( Database& database );

        void exec();
    };

    struct SelectAllFoo : BoundStatement
    {
        const Foo& foo;

        SelectAllFoo( Database& database );

        void exec();
        bool fetch();
    };

    struct InsertBar: BoundStatement
    {
        std::vector< Bar >& bar;

        InsertBar( Database& database );

        void exec();
    };

    struct SelectBarByA : BoundStatement
    {
        float& a;
        const std::vector< Bar >& bar;

        SelectBarByA( Database& database, const std::size_t batchSize );

        void exec();
        bool fetch();
    };
};

}
