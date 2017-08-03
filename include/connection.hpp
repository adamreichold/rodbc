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

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace rodbc
{

/**
 * @brief The Environment class
 */
class Environment
{
public:
    Environment();
    ~Environment();

private:
    void* env_;

    friend class Connection;
};

enum class DBMS
{
    Other,
    SQLite,
    PostgreSQL,
    MySQL
};

enum class IsolationLevel
{
    Other,
    ReadUncommitted,
    ReadCommitted,
    RepeatableRead,
    Serializable
};

/**
 * @brief The Connection class
 */
class Connection : private boost::noncopyable
{
public:
    Connection( Environment& env, const char* const connStr );
    ~Connection();

    DBMS dbms() const;

    IsolationLevel isolationLevel() const;
    void setIsolationLevel( const IsolationLevel isolationLevel );

    bool isDead() const;

private:
    void* dbc_;

    mutable boost::optional< DBMS > dbms_;

    friend class Transaction;
    friend class Statement;
};

/**
 * @brief The Transaction class
 */
class Transaction : private boost::noncopyable
{
public:
    Transaction( Connection& conn );
    ~Transaction();

    Transaction( Transaction&& );
    Transaction& operator=( Transaction&& );

    void commit();

private:
    void* dbc_;
};

}
