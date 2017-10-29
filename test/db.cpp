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

#include "connection_pool.ipp"
#include "table.ipp"
#include "typed_statement.ipp"

namespace foobar
{

struct Database::Stmts
{
    rodbc::CreateTable< Foo > createFoo;
    rodbc::CreateTable< Bar > createBar;

    rodbc::TypedStatement< Foo, std::tuple<> > insertFoo;
    rodbc::TypedStatement< std::tuple<>, Foo > selectAllFoo;

    rodbc::TypedStatement< std::vector< Bar >, std::tuple<> > insertBar;
    rodbc::TypedStatement< std::tuple< float >, std::vector< Bar > > selectBarByA;

    explicit Stmts( rodbc::Connection& conn )
    : createFoo{ conn, "foo", { "x", "y", "z" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , createBar{ conn, "bar", { "a", "b", "c" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , insertFoo{ conn, "INSERT INTO foo (x, y, z) VALUES (?, ?, ?)" }
    , selectAllFoo{ conn, "SELECT x, y, z FROM foo" }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?)" }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?", 128 }
    {
    }
};

Database::Database( const char* const connStr )
: pool_{ connStr }
{
}

Database::Transaction::Transaction( Database& database )
: database{ database }
, lease{ database.pool_ }
, transaction{ lease( []( rodbc::Connection& conn ) { return rodbc::Transaction{ conn }; } ) }
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
    lease( [ this ]() { transaction.commit(); } );

    database.stats_.committedTransactions++;
}

Database::Statement::Statement( Transaction& transaction )
: database{ transaction.database }
, lease{ transaction.lease }
, stmts{ *lease( []( Stmts& stmts ) { return &stmts; } ) }
{
    database.stats_.statements++;
    database.stats_.activeStatements++;
}

Database::Statement::~Statement()
{
    database.stats_.activeStatements--;
}

template< typename Stmt >
void Database::Statement::doExec( Stmt& stmt )
{
    lease( [ &stmt ]() { stmt.exec(); } );
}

template< typename Stmt >
bool Database::Statement::doFetch( Stmt& stmt )
{
    return lease( [ &stmt ]() { return stmt.fetch(); } );
}

Database::InsertFoo::InsertFoo( Transaction& transaction )
: Statement{ transaction }
, foo{ stmts.insertFoo.params() }
{
    database.stats_.insertFoo++;
}

void Database::InsertFoo::exec()
{
    doExec( stmts.insertFoo );
}

Database::SelectAllFoo::SelectAllFoo( Transaction& transaction )
: Statement{ transaction }
, foo{ stmts.selectAllFoo.cols() }
{
    database.stats_.selectAllFoo++;
}

void Database::SelectAllFoo::exec()
{
    doExec( stmts.selectAllFoo );
}

bool Database::SelectAllFoo::fetch()
{
    return doFetch( stmts.selectAllFoo );
}

Database::InsertBar::InsertBar( Transaction& transaction )
: Statement{ transaction }
, bar{ stmts.insertBar.params() }
{
    database.stats_.insertBar++;
}

void Database::InsertBar::exec()
{
    doExec( stmts.insertBar );
}

Database::SelectBarByA::SelectBarByA( Transaction& transaction )
: Statement{ transaction }
, a{ std::get< 0 >( stmts.selectBarByA.params() ) }
, bar{ stmts.selectBarByA.cols() }
{
    database.stats_.selectBarByA++;
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

struct Statements
{
    rodbc::Table< std::tuple< int, int, int > > foo;

    rodbc::CreateTable< std::tuple< float, float, float > > createBar;
    rodbc::TypedStatement< std::vector< std::tuple< float, float, float > >, std::tuple<> > insertBar;
    rodbc::TypedStatement< std::tuple< float >, std::vector< std::tuple< float, float, float > > > selectBarByA;

    explicit Statements( rodbc::Connection& conn )
    : foo{ conn, "foo", { "x", "y", "z" } }
    , createBar{ conn, "bar", { "a", "b", "c" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?)" }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?", 128 }
    {
        foo.create( rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE );
    }
};

using ConnectionPool = rodbc::FixedSizeConnectionPool< Statements >;

class TransactionImpl final : public Transaction
{
public:
    TransactionImpl( ConnectionPool& pool );

    void commit() override;

    template< typename Action >
    void withLease( Action action );

private:
    ConnectionPool::Lease lease;
    rodbc::Transaction transaction;
};

class DatabaseImpl final : public Database
{
public:
    DatabaseImpl( const char* const connStr, const std::size_t connPoolSize );

    std::unique_ptr< Transaction > startTransaction() override;

    void insertFoo( Transaction& transaction, const Foo& foo ) override;
    std::vector< Foo > selectAllFoo( Transaction& transaction ) override;
    void insertBar( Transaction& transaction, const std::vector< Bar >& bars ) override;
    std::vector< Bar > selectBarByA( Transaction& transaction, const float a ) override;

private:
    ConnectionPool pool_;
};

TransactionImpl::TransactionImpl(ConnectionPool &pool)
: lease{ pool }
, transaction{ lease( []( rodbc::Connection& conn ) { return rodbc::Transaction{ conn }; } ) }
{
}

void TransactionImpl::commit()
{
    lease( [ this ]() { transaction.commit(); } );
}

template< typename Action >
void TransactionImpl::withLease( Action action )
{
    lease( action );
}

template< typename Action >
void withLease( Transaction& transaction, Action&& action )
{
    static_cast< TransactionImpl& >( transaction ).withLease( std::forward< Action >( action ) );
}

DatabaseImpl::DatabaseImpl( const char* const connStr, const std::size_t connPoolSize )
: pool_{ connStr, connPoolSize }
{
}

std::unique_ptr< Transaction > DatabaseImpl::startTransaction()
{
    return std::unique_ptr< Transaction >{ new TransactionImpl{ pool_ } };
}

void DatabaseImpl::insertFoo( Transaction& transaction, const Foo& foo )
{
    withLease( transaction, [&]( Statements& stmts )
    {
        stmts.foo.insert( std::make_tuple( foo.x, foo.y, foo.z ) );
    } );
}

std::vector< Foo > DatabaseImpl::selectAllFoo( Transaction& transaction )
{
    std::vector< Foo > foos;

    withLease( transaction, [&]( Statements& stmts )
    {
        const auto rows = stmts.foo.selectAll();

        for ( const auto& row : rows )
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

void DatabaseImpl::insertBar( Transaction& transaction, const std::vector< Bar >& bars )
{
    withLease( transaction, [&]( Statements& stmts )
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

std::vector< Bar > DatabaseImpl::selectBarByA( Transaction& transaction, const float a )
{
    std::vector< Bar > bars;

    withLease( transaction, [&]( Statements& stmts )
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

std::unique_ptr< Database > makeDatabase( const char* const connStr, const std::size_t connPoolSize )
{
    return std::unique_ptr< Database >( new DatabaseImpl{ connStr, connPoolSize } );
}

}
