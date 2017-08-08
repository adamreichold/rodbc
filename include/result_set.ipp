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

template< typename Stmt, typename Cols >
inline ResultSetIterator< Stmt, Cols >::ResultSetIterator()
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

}

namespace std
{

template< typename Params, typename Cols >
inline rodbc::ResultSetIterator< rodbc::TypedStatement< Params, Cols >, Cols > begin( rodbc::TypedStatement< Params, Cols >& stmt )
{
    return { stmt };
}

template< typename Params, typename Cols >
inline rodbc::ResultSetIterator< rodbc::TypedStatement< Params, Cols >, Cols > end( rodbc::TypedStatement< Params, Cols >& )
{
    return {};
}

template< typename StagedParams, typename Params, typename Cols >
inline rodbc::ResultSetIterator< rodbc::StagedStatement< StagedParams, Params, Cols >, Cols > begin( rodbc::StagedStatement< StagedParams, Params, Cols >& stmt )
{
    return { stmt };
}

template< typename StagedParams, typename Params, typename Cols >
inline rodbc::ResultSetIterator< rodbc::StagedStatement< StagedParams, Params, Cols >, Cols > end( rodbc::StagedStatement< StagedParams, Params, Cols >& )
{
    return {};
}

}
