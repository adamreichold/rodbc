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

#include "connection_pool.hpp"

#include <boost/fusion/include/define_struct.hpp>

#include <atomic>

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

class Database
{
public:
    Database( const char* const connStr );

private:
    struct Stmts;

    using ConnectionPool = rodbc::ThreadLocalConnectionPool< Stmts >;

    ConnectionPool pool_;

    class Statement;

public:
    class Transaction
    {
    public:
        Transaction( Database& database );
        ~Transaction();

        void commit();

    private:
        Database& database;
        ConnectionPool::Lease lease;
        rodbc::Transaction transaction;

        friend class Statement;
    };

private:
    class Statement
    {
    protected:
        Statement( Transaction& transaction );
        ~Statement();

        template< typename Stmt >
        void doExec( Stmt& stmt );
        template< typename Stmt >
        bool doFetch( Stmt& stmt );

        Database& database;
        ConnectionPool::Lease& lease;
        Stmts& stmts;
    };

public:
    struct InsertFoo : private Statement
    {
        Foo& foo;

        InsertFoo( Transaction& transaction );

        void exec();
    };

    struct SelectAllFoo : private Statement
    {
        const Foo& foo;

        SelectAllFoo( Transaction& transaction );

        void exec();
        bool fetch();
    };

    struct InsertBar : private Statement
    {
        std::vector< Bar >& bar;

        InsertBar( Transaction& transaction );

        void exec();
    };

    struct SelectBarByA : private Statement
    {
        float& a;
        const std::vector< Bar >& bar;

        SelectBarByA( Transaction& transaction );

        void exec();
        bool fetch();
    };

public:
    struct Stats
    {
        std::atomic_size_t transactions;
        std::atomic_size_t committedTransactions;
        std::atomic_int activeTransactions;

        std::atomic_size_t statements;
        std::atomic_int activeStatements;

        std::atomic_size_t insertFoo;
        std::atomic_size_t selectAllFoo;
        std::atomic_size_t insertBar;
        std::atomic_size_t selectBarByA;
    };

    const Stats& stats() const noexcept;

private:
    Stats stats_;
};

}

namespace barfoo
{

struct Foo
{
    int x, y, z;
};

struct Bar
{
    float a, b, c;
};

struct Transaction
{
    virtual ~Transaction();

    virtual void commit() = 0;
};

struct Database
{
    virtual ~Database();

    virtual std::unique_ptr< Transaction > startTransaction() = 0;

    virtual void insertFoo( Transaction& transaction, const Foo& foo ) = 0;
    virtual std::vector< Foo > selectAllFoo( Transaction& transaction ) = 0;
    virtual void insertBar( Transaction& transaction, const std::vector< Bar >& bar ) = 0;
    virtual std::vector< Bar > selectBarByA( Transaction& transaction, const float a ) = 0;
};

std::unique_ptr< Database > makeDatabase( const char* const connStr, const std::size_t connPoolSize );

}
