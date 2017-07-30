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

#include "types.hpp"

#include <boost/noncopyable.hpp>

#include <vector>

namespace rodbc
{

class Connection;

class Statement : private boost::noncopyable
{
public:
    Statement( Connection& conn, const char* const stmt );
    ~Statement();

public:
    Statement& bindParam( const std::int16_t& param );
    Statement& bindParam( const std::int32_t& param );
    Statement& bindParam( const std::int64_t& param );

    Statement& bindParam( const std::uint16_t& param );
    Statement& bindParam( const std::uint32_t& param );
    Statement& bindParam( const std::uint64_t& param );

    Statement& bindParam( const float& param );
    Statement& bindParam( const double& param );

    Statement& bindParam( const bool& param );

    Statement& bindParam( const Timestamp& param );

    Statement& bindParam( const Nullable< std::int16_t >& param );
    Statement& bindParam( const Nullable< std::int32_t >& param );
    Statement& bindParam( const Nullable< std::int64_t >& param );

    Statement& bindParam( const Nullable< std::uint16_t >& param );
    Statement& bindParam( const Nullable< std::uint32_t >& param );
    Statement& bindParam( const Nullable< std::uint64_t >& param );

    Statement& bindParam( const Nullable< float >& param );
    Statement& bindParam( const Nullable< double >& param );

    Statement& bindParam( const Nullable< bool >& param );

    Statement& bindParam( const Nullable< Timestamp >& param );

    template< std::size_t Size >
    Statement& bindParam( const String< Size >& param );

    template< typename Params >
    void bindParamArray( const std::vector< Params >& params );

    Statement& rebindParams();

public:
    Statement& bindCol( std::int16_t& col );
    Statement& bindCol( std::int32_t& col );
    Statement& bindCol( std::int64_t& col );

    Statement& bindCol( std::uint16_t& col );
    Statement& bindCol( std::uint32_t& col );
    Statement& bindCol( std::uint64_t& col );

    Statement& bindCol( float& col );
    Statement& bindCol( double& col );

    Statement& bindCol( bool& col );

    Statement& bindCol( Timestamp& col );

    template< std::size_t Size >
    Statement& bindCol( String< Size >& col );

    Statement& bindCol( Nullable< std::int16_t >& col );
    Statement& bindCol( Nullable< std::int32_t >& col );
    Statement& bindCol( Nullable< std::int64_t >& col );

    Statement& bindCol( Nullable< std::uint16_t >& col );
    Statement& bindCol( Nullable< std::uint32_t >& col );
    Statement& bindCol( Nullable< std::uint64_t >& col );

    Statement& bindCol( Nullable< float >& col );
    Statement& bindCol( Nullable< double >& col );

    Statement& bindCol( Nullable< bool >& col );

    Statement& bindCol( Nullable< Timestamp >& col );

    template< std::size_t Size >
    Statement& bindCol( Nullable< String< Size > >& col );

    template< typename Cols >
    void bindColArray( std::vector< Cols >& cols, long& rowsFetched );

    Statement& rebindCols();

public:
    Statement& bindParam( std::int16_t&& ) = delete;
    Statement& bindParam( std::int32_t&& ) = delete;
    Statement& bindParam( std::int64_t&& ) = delete;

    Statement& bindParam( std::uint16_t&& ) = delete;
    Statement& bindParam( std::uint32_t&& ) = delete;
    Statement& bindParam( std::uint64_t&& ) = delete;

    Statement& bindParam( float&& ) = delete;
    Statement& bindParam( double&& ) = delete;

    Statement& bindParam( bool&& ) = delete;

    Statement& bindParam( Timestamp&& ) = delete;

    Statement& bindParam( Nullable< std::int16_t >&& ) = delete;
    Statement& bindParam( Nullable< std::int32_t >&& ) = delete;
    Statement& bindParam( Nullable< std::int64_t >&& ) = delete;

    Statement& bindParam( Nullable< std::uint16_t >&& ) = delete;
    Statement& bindParam( Nullable< std::uint32_t >&& ) = delete;
    Statement& bindParam( Nullable< std::uint64_t >&& ) = delete;

    Statement& bindParam( Nullable< float >&& ) = delete;
    Statement& bindParam( Nullable< double >&& ) = delete;

    Statement& bindParam( Nullable< bool >&& ) = delete;

    Statement& bindParam( Nullable< Timestamp >&& ) = delete;

    template< std::size_t Size >
    Statement& bindParam( String< Size >&& ) = delete;

    template< typename Params >
    void bindParamArray( std::vector< Params >&& params ) = delete;

public:
    void exec();
    bool fetch();

private:
    void* stmt_;
    std::size_t param_{ 0 };
    std::size_t col_{ 0 };

    Statement& doBindStringParam( const char* const data, const std::size_t length, const long* const indicator );
    Statement& doBindStringCol( char* const data, const std::size_t length, long* const indicator );

    void doBindParamArray( const std::size_t size, const std::size_t count );
    void doBindColArray( const std::size_t size, const std::size_t count, long* const rowsFetched );

    template< typename Param >
    Statement& doBindParam( const Param* const data, const long* const indicator = nullptr );
    template< typename Col >
    Statement& doBindCol( Col* const data, long* const indicator = nullptr );

    Statement& doBindParam( const void* const data, const short cType, const short sqlType, const std::size_t size, const std::size_t length, const long* const indicator );
    Statement& doBindCol( void* const data, const short cType, const std::size_t size, long* const indicator );
};

template< std::size_t Size >
inline Statement& Statement::bindParam( const String< Size >& param )
{
    return doBindStringParam( param.val_, Size, &param.ind_ );
}

template< std::size_t Size >
inline Statement& Statement::bindCol( String< Size >& col )
{
    return doBindStringCol( col.val_, Size, &col.ind_ );
}

template< typename Params >
inline void Statement::bindParamArray( const std::vector< Params >& params )
{
    doBindParamArray( sizeof( Params ), params.size() );
}

template< typename Cols >
inline void Statement::bindColArray( std::vector< Cols >& cols , long& rowsFetched )
{
    doBindColArray( sizeof( Cols ), cols.size(), &rowsFetched );
}

}
