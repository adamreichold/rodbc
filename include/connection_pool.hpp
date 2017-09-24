#pragma once

#include "connection.hpp"

#include <boost/noncopyable.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

#include <vector>

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

struct ConnectionPoolHolderBase : private boost::noncopyable
{
    virtual ~ConnectionPoolHolderBase();
};

class ThreadLocalConnectionPoolImpl
{
protected:
    ThreadLocalConnectionPoolImpl();

    class LeaseImpl
    {
    protected:
        LeaseImpl( ThreadLocalConnectionPoolImpl& impl );

        ConnectionPoolHolderBase* get() const;
        void reset( ConnectionPoolHolderBase* const holder = nullptr );

    private:
        ThreadLocalConnectionPoolImpl& impl_;
    };

private:
    boost::thread_specific_ptr< ConnectionPoolHolderBase > holder_;
};

class FixedSizeConnectionPoolImpl
{
protected:
    FixedSizeConnectionPoolImpl( const std::size_t size );

    class LeaseImpl
    {
    protected:
        LeaseImpl( FixedSizeConnectionPoolImpl& impl );
        ~LeaseImpl();

        ConnectionPoolHolderBase* get() const;
        void reset( ConnectionPoolHolderBase* const holder = nullptr );

    private:
        FixedSizeConnectionPoolImpl& impl_;
        std::unique_ptr< ConnectionPoolHolderBase > holder_;
    };

private:
    boost::mutex holder_lock_;
    boost::condition_variable holder_condition_;
    std::vector< std::unique_ptr< ConnectionPoolHolderBase > > holder_;
};

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

        template< typename Action >
        typename std::result_of< Action() >::type operator() ( Action action );

    private:
        ConnectionPool& pool_;
    };
};

}

/**
 * @brief The ThreadLocalConnectionPool class template
 */
template< typename Statements >
using ThreadLocalConnectionPool = detail::ConnectionPool< Statements, detail::ThreadLocalConnectionPoolImpl >;

/**
 * @brief The FixedSizeConnectionPool class template
 */
template< typename Statements >
using FixedSizeConnectionPool = detail::ConnectionPool< Statements, detail::FixedSizeConnectionPoolImpl >;

}
