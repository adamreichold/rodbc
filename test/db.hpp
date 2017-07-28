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
