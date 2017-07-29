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
#include "db.hpp"

#include "database.ipp"
#include "typed_statement.hpp"

#include "util.hpp"

#include <boost/fusion/include/std_tuple.hpp>

namespace
{

struct CreateTables
{
    CreateTables( rodbc::Connection& conn )
    {
        createTable( conn, "foo", "x INT, y INT, z INT" );
        createTable( conn, "bar", "a FLOAT, b FLOAT, c FLOAT" );
    }
};

}

namespace foobar
{

struct Statements
{
    Foo foo;
    std::vector< Bar > bar;
    std::tuple< float > a;

    CreateTables tables;

    rodbc::TypedStatement< Foo, rodbc::None > insertFoo;
    rodbc::TypedStatement< rodbc::None, Foo > selectAllFoo;

    rodbc::TypedStatement< std::vector< Bar >, rodbc::None > insertBar;
    rodbc::TypedStatement< decltype ( a ), std::vector< Bar > > selectBarByA;

    Statements( rodbc::Connection& conn )
    : tables{ conn }
    , insertFoo{ conn, "INSERT INTO foo (x, y, z) VALUES (?, ?, ?);", foo }
    , selectAllFoo{ conn, "SELECT x, y, z FROM foo;", foo }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?);", bar }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?;", a, bar }
    {
    }
};

Database::Database( const char* const connStr )
: rodbc::Database< Statements >{ connStr }
{
}

Database::Transaction::Transaction( Database& database )
: BoundTransaction{ database }
{
}

void Database::Transaction::commit()
{
    doCommit();
}

Database::InsertFoo::InsertFoo( Database& database )
: BoundStatement{ database }
, foo{ stmts_.foo }
{
}

void Database::InsertFoo::exec()
{
    doExec( stmts_.insertFoo );
}

Database::SelectAllFoo::SelectAllFoo( Database& database )
: BoundStatement{ database }
, foo{ stmts_.foo }
{
}

void Database::SelectAllFoo::exec()
{
    doExec( stmts_.selectAllFoo );
}

bool Database::SelectAllFoo::fetch()
{
    return doFetch( stmts_.selectAllFoo );
}

Database::InsertBar::InsertBar( Database& database )
: BoundStatement{ database }
, bar{ stmts_.bar }
{
}

void Database::InsertBar::exec()
{
    doExec( stmts_.insertBar );
}

Database::SelectBarByA::SelectBarByA( Database& database, const std::size_t batchSize )
: BoundStatement{ database }
, a{ std::get< 0 >( stmts_.a ) }
, bar{ stmts_.bar }
{
    resizeRowSet( stmts_.bar, batchSize );
}

void Database::SelectBarByA::exec()
{
    doExec( stmts_.selectBarByA );
}

bool Database::SelectBarByA::fetch()
{
    return doFetch( stmts_.selectBarByA );
}

}

namespace barfoo
{

Transaction::~Transaction() = default;

Database::~Database() = default;

struct Statements
{
    std::tuple< int, int, int > foo;
    std::vector< std::tuple< float, float, float > > bar;
    std::tuple< float > a;

    CreateTables tables;

    rodbc::TypedStatement< decltype ( foo ), rodbc::None > insertFoo;
    rodbc::TypedStatement< rodbc::None, decltype ( foo ) > selectAllFoo;

    rodbc::TypedStatement< decltype ( bar ), rodbc::None > insertBar;
    rodbc::TypedStatement< decltype ( a ), decltype ( bar ) > selectBarByA;

    Statements( rodbc::Connection& conn )
    : tables{ conn }
    , insertFoo{ conn, "INSERT INTO foo (x, y, z) VALUES (?, ?, ?);", foo }
    , selectAllFoo{ conn, "SELECT x, y, z FROM foo;", foo }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?);", bar }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?;", a, bar }
    {
    }
};

struct DatabaseImpl::TransactionImpl : Transaction, private BoundTransaction
{
    TransactionImpl( DatabaseImpl& database )
    : BoundTransaction{ database }
    {
    }

    void commit() override
    {
        doCommit();
    }
};

DatabaseImpl::DatabaseImpl( const char* const connStr )
: rodbc::Database< Statements >( connStr )
{
}

DatabaseImpl::~DatabaseImpl() = default;

std::unique_ptr< Transaction > DatabaseImpl::startTransaction()
{
    return std::unique_ptr< Transaction >{ new TransactionImpl{ *this } };
}

void DatabaseImpl::insertFoo( const Foo& foo )
{
    withStatements( [&]( Statements& stmts )
    {
        auto& stmt = stmts.insertFoo;
        auto& params = stmts.foo;

        std::get< 0 >( params ) = foo.x;
        std::get< 1 >( params ) = foo.y;
        std::get< 2 >( params ) = foo.z;

        stmt.exec();
    } );
}

std::vector< Foo > DatabaseImpl::selectAllFoo()
{
    std::vector< Foo > foos;

    withStatements( [&]( Statements& stmts )
    {
        auto& stmt = stmts.selectAllFoo;
        const auto& row = stmts.foo;

        stmt.exec();

        while ( stmt.fetch() )
        {
            foos.emplace_back();
            auto& foo = foos.back();

            foo.x = std::get< 0 >( row );
            foo.y = std::get< 1 >( row );
            foo.z = std::get< 2 >( row );
        }
    } );

    return foos;
}

void DatabaseImpl::insertBar( const std::vector< Bar >& bars )
{
    withStatements( [&]( Statements& stmts )
    {
        auto& stmt = stmts.insertBar;
        auto& params = stmts.bar;

        params.clear();
        params.reserve( bars.size() );

        for ( const auto& bar : bars )
        {
            params.emplace_back();
            auto& param = params.back();

            std::get< 0 >( param ) = bar.a;
            std::get< 1 >( param ) = bar.b;
            std::get< 2 >( param ) = bar.c;
        }

        stmt.exec();
    } );
}

std::vector< Bar > DatabaseImpl::selectBarByA( const float a )
{
    std::vector< Bar > bars;

    withStatements( [&]( Statements& stmts )
    {
        auto& stmt = stmts.selectBarByA;
        auto& param = std::get< 0 >( stmts.a );
        const auto& rows = stmts.bar;

        param = a;

        resizeRowSet( stmts.bar, 128 );

        stmt.exec();

        while ( stmt.fetch() )
        {
            bars.reserve( bars.size() + rows.size() );

            for ( const auto& row : rows )
            {
                bars.emplace_back();
                auto& bar = bars.back();

                bar.a = std::get< 0 >( row );
                bar.b = std::get< 1 >( row );
                bar.c = std::get< 2 >( row );
            }
        }
    } );

    return bars;
}

}
