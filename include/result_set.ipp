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
inline StmtIterator< Stmt, Cols >::StmtIterator( Stmt& stmt )
: stmt_( stmt )
{
}

template< typename Stmt, typename Cols >
inline bool StmtIterator< Stmt, Cols >::increment()
{
    return stmt_.fetch();
}

template< typename Stmt, typename Cols >
inline const Cols& StmtIterator< Stmt, Cols >::dereference() const
{
    return stmt_.cols();
}

template< typename Stmt, typename Cols >
inline StmtIterator< Stmt, std::vector< Cols > >::StmtIterator( Stmt& stmt )
: stmt_( stmt )
, row_{ stmt_.cols().begin() }
{
}

template< typename Stmt, typename Cols >
inline bool StmtIterator< Stmt, std::vector< Cols > >::increment()
{
    if ( ++row_ != stmt_.cols().end() )
    {
        return true;
    }
    
    if ( stmt_.fetch() )
    {
        row_ = stmt_.cols().begin();
        
        return true;
    }

    return false;
}

template< typename Stmt, typename Cols >
inline const Cols& StmtIterator< Stmt, std::vector< Cols > >::dereference() const
{
    return *row_;
}

}

template< typename Cols >
template< typename Stmt >
inline ResultSetIterator< Cols >::ResultSetIterator( const detail::StmtTag&, Stmt& stmt )
{
    stmt.exec();

    if ( stmt.fetch() )
    {
        it_.reset( new detail::StmtIterator< Stmt, typename std::decay< decltype( stmt.cols() ) >::type >{ stmt } );
    }
}

template< typename Cols >
void ResultSetIterator< Cols >::increment()
{
    if ( !it_->increment() )
    {
        it_.reset();
    }
}

template< typename Cols >
bool ResultSetIterator< Cols >::equal( const ResultSetIterator& other ) const
{
    return it_ == other.it_;
}

template< typename Cols >
const Cols& ResultSetIterator< Cols >::dereference() const
{
    return it_->dereference();
}

template< typename Cols >
template< typename Stmt >
inline ResultSet< Cols >::ResultSet( Stmt& stmt )
: begin_{ detail::StmtTag{}, stmt }
{
}

template< typename Cols >
inline const ResultSetIterator< Cols >& ResultSet< Cols >::begin() const
{
    return begin_;
}

template< typename Cols >
inline ResultSetIterator< Cols > ResultSet< Cols >::end() const
{
    return {};
}

}
