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

#include <atomic>
#include <memory>
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

struct Database : rodbc::Database< Database, Statements, rodbc::ThreadLocalConnectionPool< Statements > >
{
    Database( const char* const connStr );

    struct Stats;

    struct Transaction : BoundTransaction
    {
        Transaction( Database& database );
        ~Transaction();

        void commit();
    };

    struct InsertFoo : BoundStatement
    {
        Foo& foo;

        InsertFoo( Transaction& transaction );
        ~InsertFoo();

        void exec();
    };

    struct SelectAllFoo : BoundStatement
    {
        const Foo& foo;

        SelectAllFoo( Transaction& transaction );
        ~SelectAllFoo();

        void exec();
        bool fetch();
    };

    struct InsertBar : BoundStatement
    {
        std::vector< Bar >& bar;

        InsertBar( Transaction& transaction );
        ~InsertBar();

        void exec();
    };

    struct SelectBarByA : BoundStatement
    {
        float& a;
        const std::vector< Bar >& bar;

        SelectBarByA( Transaction& transaction );
        ~SelectBarByA();

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

struct Statements;

struct DatabaseImpl final : Database, rodbc::Database< DatabaseImpl, Statements, rodbc::FixedSizeConnectionPool< Statements > >
{
    DatabaseImpl( const char* const connStr, const std::size_t connPoolSize );
    ~DatabaseImpl();

    std::unique_ptr< Transaction > startTransaction() override;

    void insertFoo( Transaction& transaction, const Foo& foo ) override;
    std::vector< Foo > selectAllFoo( Transaction& transaction ) override;
    void insertBar( Transaction& transaction, const std::vector< Bar >& bars ) override;
    std::vector< Bar > selectBarByA( Transaction& transaction, const float a ) override;

private:
    struct TransactionImpl;

    template< typename Action >
    void withLease( Transaction& transaction, Action action );
};

}
