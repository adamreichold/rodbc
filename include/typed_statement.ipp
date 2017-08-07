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

#include "typed_statement.hpp"

#include <boost/fusion/include/flatten.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/std_tuple.hpp>

namespace rodbc
{
namespace detail
{

struct ParamBinder
{
    Statement* const stmt;

    template< typename Param >
    void operator() ( const Param& param ) const
    {
        stmt->bindParam( param );
    }
};

template< typename Params >
inline void bindParams( Statement& stmt, const Params& params )
{
    stmt.rebindParams();
    boost::fusion::for_each( boost::fusion::flatten( params ), detail::ParamBinder{ &stmt } );
}

struct ColBinder
{
    Statement* const stmt;

    template< typename Col >
    void operator() ( Col& col ) const
    {
        stmt->bindCol( col );
    }
};

template< typename Cols >
inline void bindCols( Statement& stmt, Cols& cols )
{
    stmt.rebindCols();
    boost::fusion::for_each( boost::fusion::flatten( cols ), detail::ColBinder{ &stmt } );
}

}

template< typename Params, typename Cols >
inline TypedStatement< Params, Cols >::TypedStatement( Connection& conn, const char* const stmt )
: stmt_{ conn, stmt }
{
    detail::bindParams( stmt_, params_ );
    detail::bindCols( stmt_, cols_ );
}

template< typename Params, typename Cols >
inline Params& TypedStatement< Params, Cols >::params()
{
    return params_;
}

template< typename Params, typename Cols >
inline const Cols& TypedStatement< Params, Cols >::cols() const
{
    return cols_;
}

template< typename Params, typename Cols >
inline void TypedStatement< Params, Cols >::exec()
{
    stmt_.exec();
}

template< typename Params, typename Cols >
inline bool TypedStatement< Params, Cols >::fetch()
{
    return stmt_.fetch();
}

template< typename Params >
inline TypedStatement< std::vector< Params >, std::tuple<> >::TypedStatement( Connection& conn, const char* const stmt )
: stmt_{ conn, stmt }
{
}

template<typename Params>
inline std::vector< Params >& TypedStatement< std::vector< Params >, std::tuple<> >::params()
{
    return params_;
}

template< typename Params >
inline void TypedStatement< std::vector< Params >, std::tuple<> >::exec()
{
    bindParams();

    stmt_.exec();
}

template< typename Params >
inline void TypedStatement< std::vector< Params >, std::tuple<> >::bindParams()
{
    const auto binding = std::make_pair( params_.data(), params_.size() );
    if ( binding_ == binding )
    {
        return;
    }

    stmt_.bindParamArray( params_ );
    detail::bindParams( stmt_, params_.front() );

    binding_ = binding;
}

template< typename Params, typename Cols >
inline TypedStatement< Params, std::vector< Cols > >::TypedStatement( Connection& conn, const char* const stmt, std::size_t fetchSize )
: stmt_{ conn, stmt }
{
    detail::bindParams( stmt_, params_ );

    cols_.reserve( fetchSize );
}

template< typename Params, typename Cols >
inline void TypedStatement< Params, std::vector< Cols > >::exec()
{
    cols_.resize( cols_.capacity() );

    bindCols();

    stmt_.exec();
}

template< typename Params, typename Cols >
inline bool TypedStatement< Params, std::vector< Cols > >::fetch()
{
    if ( cols_.size() != cols_.capacity() || !stmt_.fetch() )
    {
        return false;
    }

    cols_.resize( rowsFetched_ );

    return rowsFetched_ != 0;
}

template< typename Params, typename Cols >
inline void TypedStatement< Params, std::vector< Cols > >::bindCols()
{
    const auto binding = std::make_pair( cols_.data(), cols_.size() );
    if ( binding_ == binding )
    {
        return;
    }

    stmt_.bindColArray( cols_, rowsFetched_ );
    detail::bindCols( stmt_, cols_.front() );

    binding_ = binding;
}

template< typename Params, typename Cols >
inline Params& TypedStatement< Params, std::vector< Cols > >::params()
{
    return params_;
}

template< typename Params, typename Cols >
const std::vector< Cols >& TypedStatement< Params, std::vector< Cols > >::cols() const
{
    return cols_;
}

}
