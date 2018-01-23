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

#include "result_set.hpp"

namespace rodbc
{
namespace detail
{

template< typename Stmt, typename Cols >
inline constexpr ResultSetIterator< Stmt, Cols >::ResultSetIterator()
: stmt_{ nullptr }
{
}

template< typename Stmt, typename Cols >
inline ResultSetIterator< Stmt, Cols >::ResultSetIterator( Stmt& stmt )
: stmt_{ &stmt }
{
    increment();
}

template< typename Stmt, typename Cols >
inline void ResultSetIterator< Stmt, Cols >::increment()
{
    if ( !stmt_->fetch() )
    {
        stmt_ = nullptr;
    }
}

template< typename Stmt, typename Cols >
inline bool ResultSetIterator< Stmt, Cols >::equal( const ResultSetIterator& other ) const
{
    return stmt_ == other.stmt_;
}

template< typename Stmt, typename Cols >
inline const Cols& ResultSetIterator< Stmt, Cols >::dereference() const
{
    return stmt_->cols();
}

template< typename Stmt, typename Cols >
inline constexpr ResultSetIterator< Stmt, std::vector< Cols > >::ResultSetIterator()
: stmt_{ nullptr }
{
}

template< typename Stmt, typename Cols >
inline ResultSetIterator< Stmt, std::vector< Cols > >::ResultSetIterator( Stmt& stmt )
: stmt_{ &stmt }
{
    fetch();
}

template< typename Stmt, typename Cols >
inline void ResultSetIterator< Stmt, std::vector< Cols > >::increment()
{
    if ( ++it_ == stmt_->cols().end() )
    {
        fetch();
    }
}

template< typename Stmt, typename Cols >
inline bool ResultSetIterator< Stmt, std::vector< Cols > >::equal( const ResultSetIterator& other ) const
{
    if ( !stmt_ )
    {
        return !other.stmt_;
    }
    else
    {
        return stmt_ == other.stmt_ && it_ == other.it_;
    }
}

template< typename Stmt, typename Cols >
inline const Cols& ResultSetIterator< Stmt, std::vector< Cols > >::dereference() const
{
    return *it_;
}

template< typename Stmt, typename Cols >
inline void ResultSetIterator< Stmt, std::vector< Cols > >::fetch()
{
    if ( !stmt_->fetch() )
    {
        stmt_ = nullptr;
    }
    else
    {
        it_ = stmt_->cols().begin();
    }
}

}

template< typename Stmt, typename Cols >
inline ResultSet< Stmt, Cols >::ResultSet( Stmt& stmt )
: stmt_( stmt )
{
    stmt_.exec();
}

template< typename Stmt, typename Cols >
inline detail::ResultSetIterator< Stmt, Cols > ResultSet< Stmt, Cols >::begin()
{
    return { stmt_ };
}

template< typename Stmt, typename Cols >
inline detail::ResultSetIterator< Stmt, Cols > ResultSet< Stmt, Cols >::end()
{
    return {};
}

template< typename Stmt >
inline auto resultsOf( Stmt& stmt ) -> ResultSet< Stmt, typename std::decay< decltype( stmt.cols() ) >::type >
{
    return { stmt };
}

}
