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

#include "create_table.ipp"
#include "database.ipp"
#include "typed_statement.ipp"

namespace foobar
{

struct Database::Statements
{
    Database::Stats& stats;

    rodbc::CreateTable< Foo > createFoo;
    rodbc::CreateTable< Bar > createBar;

    rodbc::TypedStatement< Foo, std::tuple<> > insertFoo;
    rodbc::TypedStatement< std::tuple<>, Foo > selectAllFoo;

    rodbc::TypedStatement< std::vector< Bar >, std::tuple<> > insertBar;
    rodbc::TypedStatement< std::tuple< float >, std::vector< Bar > > selectBarByA;

    Statements( Database& database, rodbc::Connection& conn )
    : stats{ database.stats_ }
    , createFoo{ conn, "foo", { "x", "y", "z" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , createBar{ conn, "bar", { "a", "b", "c" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , insertFoo{ conn, "INSERT INTO foo (x, y, z) VALUES (?, ?, ?)" }
    , selectAllFoo{ conn, "SELECT x, y, z FROM foo" }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?)" }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?", 128 }
    {
        stats.sessions++;
        stats.activeSessions++;
    }

    ~Statements()
    {
        stats.activeSessions--;
    }
};

Database::Database( const char* const connStr )
: rodbc::Database< Database >{ connStr }
{
}

Database::Transaction::Transaction( Database& database )
: BoundTransaction{ database }
{
    database.stats_.transactions++;
    database.stats_.activeTransactions++;
}

Database::Transaction::~Transaction()
{
    database.stats_.activeTransactions--;
}

void Database::Transaction::commit()
{
    doCommit();

    database.stats_.committedTransactions++;
}

Database::InsertFoo::InsertFoo( Database& database )
: BoundStatement{ database }
, foo{ stmts.insertFoo.params() }
{
    database.stats_.statements++;
    database.stats_.activeStatements++;
    database.stats_.insertFoo++;
}

Database::InsertFoo::~InsertFoo()
{
    database.stats_.activeStatements--;
}

void Database::InsertFoo::exec()
{
    doExec( stmts.insertFoo );
}

Database::SelectAllFoo::SelectAllFoo( Database& database )
: BoundStatement{ database }
, foo{ stmts.selectAllFoo.cols() }
{
    database.stats_.statements++;
    database.stats_.activeStatements++;
    database.stats_.selectAllFoo++;
}

Database::SelectAllFoo::~SelectAllFoo()
{
    database.stats_.activeStatements--;
}

void Database::SelectAllFoo::exec()
{
    doExec( stmts.selectAllFoo );
}

bool Database::SelectAllFoo::fetch()
{
    return doFetch( stmts.selectAllFoo );
}

Database::InsertBar::InsertBar( Database& database )
: BoundStatement{ database }
, bar{ stmts.insertBar.params() }
{
    database.stats_.statements++;
    database.stats_.activeStatements++;
    database.stats_.insertBar++;
}

Database::InsertBar::~InsertBar()
{
    database.stats_.activeStatements--;
}

void Database::InsertBar::exec()
{
    doExec( stmts.insertBar );
}

Database::SelectBarByA::SelectBarByA( Database& database )
: BoundStatement{ database }
, a{ std::get< 0 >( stmts.selectBarByA.params() ) }
, bar{ stmts.selectBarByA.cols() }
{
    database.stats_.statements++;
    database.stats_.activeStatements++;
    database.stats_.selectBarByA++;
}

Database::SelectBarByA::~SelectBarByA()
{
    database.stats_.activeStatements--;
}

void Database::SelectBarByA::exec()
{
    doExec( stmts.selectBarByA );
}

bool Database::SelectBarByA::fetch()
{
    return doFetch( stmts.selectBarByA );
}

const Database::Stats& Database::stats() const noexcept
{
    return stats_;
}

}

namespace barfoo
{

Transaction::~Transaction() = default;

Database::~Database() = default;

struct DatabaseImpl::Statements
{
    rodbc::CreateTable< std::tuple< int, int, int > > createFoo;
    rodbc::CreateTable< std::tuple< float, float, float > > createBar;

    rodbc::TypedStatement< std::tuple< int, int, int >, std::tuple<> > insertFoo;
    rodbc::TypedStatement< std::tuple<>, std::tuple< int, int, int > > selectAllFoo;

    rodbc::TypedStatement< std::vector< std::tuple< float, float, float > >, std::tuple<> > insertBar;
    rodbc::TypedStatement< std::tuple< float >, std::vector< std::tuple< float, float, float > > > selectBarByA;

    Statements( DatabaseImpl&, rodbc::Connection& conn )
    : createFoo{ conn, "foo", { "x", "y", "z" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , createBar{ conn, "bar", { "a", "b", "c" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , insertFoo{ conn, "INSERT INTO foo (x, y, z) VALUES (?, ?, ?)" }
    , selectAllFoo{ conn, "SELECT x, y, z FROM foo" }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?)" }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?", 128 }
    {
    }
};

struct DatabaseImpl::TransactionImpl final : Transaction, private BoundTransaction
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
: rodbc::Database< DatabaseImpl >( connStr )
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
        auto& params = stmt.params();

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
        const auto& cols = stmt.cols();

        stmt.exec();

        while ( stmt.fetch() )
        {
            foos.emplace_back();
            auto& foo = foos.back();

            foo.x = std::get< 0 >( cols );
            foo.y = std::get< 1 >( cols );
            foo.z = std::get< 2 >( cols );
        }
    } );

    return foos;
}

void DatabaseImpl::insertBar( const std::vector< Bar >& bars )
{
    withStatements( [&]( Statements& stmts )
    {
        auto& stmt = stmts.insertBar;
        auto& params = stmt.params();

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
        auto& param = std::get< 0 >( stmt.params() );
        const auto& rows = stmt.cols();

        param = a;

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
