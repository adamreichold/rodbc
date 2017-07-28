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

#include <boost/fusion/include/define_struct.hpp>

#include <cassert>

BOOST_FUSION_DEFINE_STRUCT( ( rodbc ), None, /**/ );

namespace rodbc
{
namespace detail
{

template< typename Type >
struct IsNone : public std::is_same< Type, None > {};
template< typename Type >
using EnableIfNone = typename std::enable_if< IsNone< Type >::value >::type;
template< typename Type >
using EnableIfNotNone = typename std::enable_if< !IsNone< Type >::value >::type;

}

template< typename Params, typename Cols >
class TypedStatement
{
public:
    template< typename DefParams = Params, typename DefCols = Cols >
    TypedStatement(
        Connection& conn,
        const char* const stmt,
        const Params& params,
        Cols& cols,
        detail::EnableIfNotNone< DefParams >* = nullptr,
        detail::EnableIfNotNone< DefCols >* = nullptr
    );

    template< typename DefParams = Params >
    TypedStatement(
        Connection& conn,
        const char* const stmt,
        Cols& cols,
        detail::EnableIfNone< DefParams >* = nullptr
    );

    template< typename DefCols = Cols >
    TypedStatement(
        Connection& conn,
        const char* const stmt,
        const Params& params,
        detail::EnableIfNone< DefCols >* = nullptr
    );

public:
    void exec();

    template< typename DefCols = Cols >
    bool fetch( detail::EnableIfNotNone< DefCols >* = nullptr );

private:
    Statement stmt_;
};

template< typename Params >
class TypedStatement< std::vector< Params >, None >
{
public:
    TypedStatement(
        Connection& conn,
        const char* const stmt,
        const std::vector< Params >& params
    );

public:
    void exec();

private:
    Statement stmt_;
    const std::vector< Params >& params_;
    std::pair< const Params*, std::size_t > binding_{ nullptr, 0 };

    void bindParams();
};

template< typename Params, typename Cols >
class TypedStatement< Params, std::vector< Cols > >
{
public:
    template< typename DefParams = Params >
    TypedStatement(
        Connection& conn,
        const char* const stmt,
        const Params& params,
        std::vector< Cols >& rows,
        detail::EnableIfNotNone< DefParams >* = nullptr
    );

    template< typename DefParams = Params >
    TypedStatement(
        Connection& conn,
        const char* const stmt,
        std::vector< Cols >& rows,
        detail::EnableIfNone< DefParams >* = nullptr
    );

public:
    void exec();
    bool fetch();

private:
    Statement stmt_;
    std::vector< Cols >& rows_;
    std::pair< Cols*, std::size_t > binding_{ nullptr, 0 };
    long rowsFetched_;

    void bindCols();
};

template< typename Params, typename Cols >
template< typename DefParams, typename DefCols >
inline TypedStatement< Params, Cols >::TypedStatement(
    Connection& conn,
    const char* const stmt,
    const Params& params,
    Cols& cols,
    detail::EnableIfNotNone< DefParams >*,
    detail::EnableIfNotNone< DefCols >*
)
: stmt_{ conn, stmt }
{
    stmt_.bindParams( params );
    stmt_.bindCols( cols );
}

template< typename Params, typename Cols >
template< typename DefParams >
inline TypedStatement< Params, Cols >::TypedStatement(
    Connection& conn,
    const char* const stmt,
    Cols& cols,
    detail::EnableIfNone< DefParams >*
)
: stmt_{ conn, stmt }
{
    stmt_.bindCols( cols );
}

template< typename Params, typename Cols >
template< typename DefCols >
inline TypedStatement< Params, Cols >::TypedStatement(
    Connection& conn,
    const char* const stmt,
    const Params& params,
    detail::EnableIfNone< DefCols >*
)
: stmt_{ conn, stmt }
{
    stmt_.bindParams( params );
}

template< typename Params, typename Cols >
inline void TypedStatement< Params, Cols >::exec()
{
    stmt_.exec();
}

template< typename Params, typename Cols >
template< typename DefCols >
inline bool TypedStatement< Params, Cols >::fetch( detail::EnableIfNotNone< DefCols >* )
{
    return stmt_.fetch();
}

template< typename Params >
inline TypedStatement< std::vector< Params >, None >::TypedStatement(
    Connection& conn,
    const char* const stmt,
    const std::vector< Params >& params
)
: stmt_{ conn, stmt }
, params_( params )
{
}

template< typename Params >
inline void TypedStatement< std::vector< Params >, None >::exec()
{
    bindParams();
    stmt_.exec();
}

template< typename Params >
inline void TypedStatement< std::vector< Params >, None >::bindParams()
{
    const auto binding = std::make_pair( params_.data(), params_.size() );

    if ( binding_ == binding )
    {
        return;
    }

    stmt_.bindParamArray( params_ );
    binding_ = binding;
}

template< typename Params, typename Cols >
template< typename DefParams >
inline TypedStatement< Params, std::vector< Cols > >::TypedStatement(
    Connection& conn,
    const char* const stmt,
    const Params& params,
    std::vector< Cols >& rows,
    detail::EnableIfNotNone< DefParams >*
)
: stmt_{ conn, stmt }
, rows_( rows )
{
    stmt_.bindParams( params );
}

template< typename Params, typename Cols >
template< typename DefParams >
inline TypedStatement< Params, std::vector< Cols > >::TypedStatement(
    Connection& conn,
    const char* const stmt,
    std::vector< Cols >& rows,
    detail::EnableIfNone< DefParams >*
)
: stmt_{ conn, stmt }
, rows_{ rows }
{
}

template< typename Params, typename Cols >
inline void TypedStatement< Params, std::vector< Cols > >::exec()
{
    assert( rows_.size() == rows_.capacity() && "Row set size and capactiy must be equals before statement execution." );

    bindCols();
    stmt_.exec();
}

template< typename Params, typename Cols >
inline bool TypedStatement< Params, std::vector< Cols > >::fetch()
{
    if ( rows_.size() != rows_.capacity() || !stmt_.fetch() )
    {
        rows_.resize( rows_.capacity() );

        return false;
    }

    rows_.resize( rowsFetched_ );

    return rowsFetched_ != 0;
}

template< typename Params, typename Cols >
inline void TypedStatement< Params, std::vector< Cols > >::bindCols()
{
    const auto binding = std::make_pair( rows_.data(), rows_.size() );

    if ( binding_ == binding )
    {
        return;
    }

    stmt_.bindColArray( rows_, rowsFetched_ );
    binding_ = binding;
}

}
