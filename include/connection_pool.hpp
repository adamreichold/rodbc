#pragma once

#include "connection.hpp"

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

namespace rodbc
{
namespace detail
{

class ConnectionPoolBase : private boost::noncopyable
{
protected:
    ConnectionPoolBase( std::string&& connStr );
    ~ConnectionPoolBase();

    Connection makeConnection();

private:
    const std::string connStr_;

    boost::mutex env_lock_;
    Environment env_;
};

template< typename Statements >
class ThreadLocalConnectionPoolImpl;

template< typename Statements >
class FixedSizeConnectionPoolImpl;

template< typename Statements, typename ConnectionPoolImpl >
class ConnectionPool : private ConnectionPoolBase, private ConnectionPoolImpl
{
public:
    template< typename... Args >
    ConnectionPool( std::string connStr, Args&&... args );

    class Lease : private ConnectionPoolImpl::LeaseImpl
    {
    public:
        Lease( ConnectionPool& pool );

        template< typename Action >
        typename std::result_of< Action( Connection&, Statements& ) >::type operator() ( Action action );

        template< typename Action >
        typename std::result_of< Action( Connection& ) >::type operator() ( Action action );

        template< typename Action >
        typename std::result_of< Action( Statements& ) >::type operator() ( Action action );

    private:
        ConnectionPool& pool_;
    };
};

}

/**
 * @brief The ThreadLocalConnectionPool class template
 */
template< typename Statements >
using ThreadLocalConnectionPool = detail::ConnectionPool< Statements, detail::ThreadLocalConnectionPoolImpl< Statements > >;

/**
 * @brief The FixedSizeConnectionPool class template
 */
template< typename Statements >
using FixedSizeConnectionPool = detail::ConnectionPool< Statements, detail::FixedSizeConnectionPoolImpl< Statements > >;

}
