#pragma once
#include "connection_pool.hpp"

#include "types.hpp"

namespace rodbc
{
namespace detail
{

template< typename Statements >
struct ConnectionPoolHolder final : public ConnectionPoolHolderBase
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
inline ConnectionPool< Statements, ConnectionPoolImpl >::ConnectionPool( const char* const connStr, Args&&... args )
: ConnectionPoolBase{ connStr }
, ConnectionPoolImpl{ std::forward< Args >( args )... }
{
}

template< typename Statements, typename ConnectionPoolImpl >
inline ConnectionPool< Statements, ConnectionPoolImpl >::Lease::Lease( ConnectionPool& pool )
: ConnectionPoolImpl::LeaseImpl{ pool }
, pool_{ pool }
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
            Impl::reset();
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
