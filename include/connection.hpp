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

namespace rodbc
{

class Connection : private boost::noncopyable
{
public:
    Connection( const char* const connStr );
    ~Connection();

    bool isDead() const;

private:
    void* env_;
    void* dbc_;

    friend class Transaction;
    friend class Statement;
};

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
