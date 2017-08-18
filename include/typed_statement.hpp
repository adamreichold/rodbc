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

#include "statement.hpp"

namespace rodbc
{

/**
 * @brief The TypedStatement class template
 */
template< typename Params, typename Cols >
class TypedStatement
{
public:
    TypedStatement( Connection& conn, const char* const stmt );

    Params& params();
    const Cols& cols() const;

public:
    void exec();
    bool fetch();

private:
    Statement stmt_;
    Params params_;
    Cols cols_;
};

template< typename Params >
class TypedStatement< std::vector< Params >, std::tuple<> >
{
public:
    TypedStatement( Connection& conn, const char* const stmt );

    std::vector< Params >& params();

public:
    void exec();

private:
    Statement stmt_;

    std::vector< Params > params_;
    Params* data_{ nullptr };
    std::size_t size_{ 0 };

    void bindParams();
};

template< typename Params, typename Cols >
class TypedStatement< Params, std::vector< Cols > >
{
public:
    TypedStatement( Connection& conn, const char* const stmt, const std::size_t fetchSize );

    Params& params();
    const std::vector< Cols >& cols() const;

    std::size_t fetchSize() const;
    void setFetchSize( const std::size_t fetchSize );

public:
    void exec();
    bool fetch();

private:
    Statement stmt_;
    Params params_;

    std::vector< Cols > cols_;
    Cols* data_{ nullptr };
    std::size_t size_{ 0 };

    long rowsFetched_;

    void bindCols();
};

}
