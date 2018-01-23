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
#include "connection_pool.hpp"

#include "types.hpp"

namespace rodbc
{
namespace detail
{

template< typename Statements >
struct ConnectionPoolHolder final : ConnectionPoolHolderBase
{
    ConnectionPoolHolder( Connection&& conn )
    : conn{ std::move( conn ) }
    , stmts{ this->conn }
    {
    }

    Connection conn;
    Statements stmts;
};

template< typename Statements, typename ConnectionPoolImpl >
template< typename... Args >
inline ConnectionPool< Statements, ConnectionPoolImpl >::ConnectionPool( std::string connStr, Args&&... args )
: ConnectionPoolBase{ std::move( connStr ) }
, ConnectionPoolImpl{ std::forward< Args >( args )... }
{
}

template< typename Statements, typename ConnectionPoolImpl >
inline ConnectionPool< Statements, ConnectionPoolImpl >::Lease::Lease( ConnectionPool& pool )
: ConnectionPoolImpl::LeaseImpl{ pool }
, pool_( pool )
{
}

template< typename Statements, typename ConnectionPoolImpl >
template< typename Action >
inline typename std::result_of< Action( Connection&, Statements& ) >::type ConnectionPool< Statements, ConnectionPoolImpl >::Lease::operator() ( Action action )
{
    using Impl = typename ConnectionPoolImpl::LeaseImpl;
    using Holder = ConnectionPoolHolder< Statements >;

    auto* data = static_cast< Holder* >( Impl::get() );

    if ( !data )
    {
        data = new Holder{ pool_.makeConnection() };

        Impl::reset( data );
    }

    try
    {
        return action( data->conn, data->stmts );
    }
    catch ( Exception& )
    {
        if ( data->conn.isDead() )
        {
            Impl::reset( nullptr );
        }

        throw;
    }
}

template< typename Statements, typename ConnectionPoolImpl >
template< typename Action >
inline typename std::result_of< Action( Connection& ) >::type ConnectionPool< Statements, ConnectionPoolImpl >::Lease::operator() ( Action action )
{
    return operator() ( [ &action ]( Connection& conn, Statements& )
    {
        return action( conn );
    } );
}

template< typename Statements, typename ConnectionPoolImpl >
template< typename Action >
inline typename std::result_of< Action( Statements& ) >::type ConnectionPool< Statements, ConnectionPoolImpl >::Lease::operator() ( Action action )
{
    return operator() ( [ &action ]( Connection&, Statements& stmts )
    {
        return action( stmts );
    } );
}

template< typename Statements, typename ConnectionPoolImpl >
template< typename Action >
inline typename std::result_of< Action() >::type ConnectionPool< Statements, ConnectionPoolImpl >::Lease::operator() ( Action action )
{
    return operator() ( [ &action ]( Connection&, Statements& )
    {
        return action();
    } );
}

}
}
