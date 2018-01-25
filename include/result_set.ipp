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

template< typename Cols >
inline constexpr ResultSetIterator< Cols >::ResultSetIterator()
: cols_{ nullptr }
{
}

template< typename Cols >
template< typename Stmt >
inline ResultSetIterator< Cols >::ResultSetIterator( Stmt& stmt )
: cols_{ &stmt.cols() }
, fetch_{ [ &stmt ]() { return stmt.fetch(); } }
{
    increment();
}

template< typename Cols >
inline void ResultSetIterator< Cols >::increment()
{
    if ( !fetch_() )
    {
        cols_ = nullptr;
    }
}

template< typename Cols >
inline bool ResultSetIterator< Cols >::equal( const ResultSetIterator& other ) const
{
    return cols_ == other.cols_;
}

template< typename Cols >
inline const Cols& ResultSetIterator< Cols >::dereference() const
{
    return *cols_;
}

template< typename Cols >
inline constexpr ResultSetIterator< std::vector< Cols > >::ResultSetIterator()
: cols_{ nullptr }
{
}

template< typename Cols >
template< typename Stmt >
inline ResultSetIterator< std::vector< Cols > >::ResultSetIterator( Stmt& stmt )
: cols_{ &stmt.cols() }
, fetch_{ [ &stmt ]() { return stmt.fetch(); } }
{
    fetch();
}

template< typename Cols >
inline void ResultSetIterator< std::vector< Cols > >::increment()
{
    if ( ++it_ == cols_->end() )
    {
        fetch();
    }
}

template< typename Cols >
inline bool ResultSetIterator< std::vector< Cols > >::equal( const ResultSetIterator& other ) const
{
    if ( !cols_ )
    {
        return !other.cols_;
    }
    else
    {
        return cols_ == other.cols_ && it_ == other.it_;
    }
}

template< typename Cols >
inline const Cols& ResultSetIterator< std::vector< Cols > >::dereference() const
{
    return *it_;
}

template< typename Cols >
inline void ResultSetIterator< std::vector< Cols > >::fetch()
{
    if ( !fetch_() )
    {
        cols_ = nullptr;
    }
    else
    {
        it_ = cols_->begin();
    }
}

template< typename Stmt >
inline ExecStmt::ExecStmt( Stmt& stmt )
{
    stmt.exec();
}

}

template< typename Cols >
template< typename Stmt >
inline ResultSet< Cols >::ResultSet( Stmt& stmt )
: ExecStmt{ stmt }
, begin_{ stmt }
{
}

template< typename Cols >
inline const typename ResultSet< Cols >::Iterator& ResultSet< Cols >::begin() const
{
    return begin_;
}

template< typename Cols >
inline constexpr typename ResultSet< Cols >::Iterator ResultSet< Cols >::end() const
{
    return {};
}

}
