#pragma once
#include "connection_pool.hpp"

#include "types.hpp"

#include <boost/scope_exit.hpp>
#include <boost/thread/lock_types.hpp>

namespace rodbc
{
namespace detail
{

template< typename Statements >
struct ConnectionPoolData
{
    ConnectionPoolData( Connection&& conn )
    : conn{ std::move( conn ) }
    , stmts{ this->conn }
    {
    }

    Connection conn;
    Statements stmts;
};

template< typename Statements >
class ThreadLocalConnectionPoolImpl
{
private:
    using Impl = ThreadLocalConnectionPoolImpl;
    using Data = ConnectionPoolData< Statements >;

    boost::thread_specific_ptr< ConnectionPoolData< Statements > > data_;

protected:
    class Lease
    {
    private:
        Impl& impl_;

    public:
        Lease( Impl& impl )
        : impl_{ impl }
        {
        }

        Data* get() const
        {
            return impl_.data_.get();
        }

        void reset( Data* const data = nullptr )
        {
            impl_.data_.reset( data );
        }
    };

    ThreadLocalConnectionPoolImpl() = default;
};

template< typename Statements >
class FixedSizeConnectionPoolImpl
{
private:
    using Impl = FixedSizeConnectionPoolImpl;
    using Data = ConnectionPoolData< Statements >;

    boost::mutex data_lock_;
    boost::condition_variable data_condition_;
    std::vector< std::unique_ptr< detail::ConnectionPoolData< Statements > > > data_;

protected:
    class Lease
    {
    private:
        Impl& impl_;
        std::unique_ptr< Data > data_;

    public:
        Lease( Impl& impl )
        : impl_{ impl }
        {
            boost::unique_lock< boost::mutex > lock{ impl_.data_lock_ };

            while ( impl_.data_.empty() )
            {
                impl_.data_condition_.wait( lock );
            }

            data_ = std::move( impl_.data_.back() );
            impl_.data_.pop_back();
        }

        ~Lease()
        {
            boost::unique_lock< boost::mutex > lock{ impl_.data_lock_ };

            impl_.data_.push_back( std::move( data_ ) );

            impl_.data_condition_.notify_one();
        }

        Data* get()
        {
            return data_.get();
        }

        void reset( Data* const data = nullptr )
        {
            data_.reset( data );
        }
    };

    FixedSizeConnectionPoolImpl( const std::size_t size )
    : data_{ size }
    {
    }
};


template< typename Statements, typename ConnectionPoolImpl >
template< typename... Args >
inline ConnectionPool< Statements, ConnectionPoolImpl >::ConnectionPool( std::string connStr, Args&&... args )
: ConnectionPoolBase{ std::move( connStr ) }
, ConnectionPoolImpl{ std::forward< Args >( args )... }
{
}

template< typename Statements, typename ConnectionPoolImpl >
template< typename Action >
inline auto ConnectionPool< Statements, ConnectionPoolImpl >::operator() ( Action action ) -> typename std::result_of< Action( Connection&, Statements& ) >::type
{
    typename ConnectionPoolImpl::Lease lease{ *this };

    auto* data = lease.get();

    if ( !data )
    {
        data = new ConnectionPoolData< Statements >{ makeConnection() };

        lease.reset( data );
    }

    try
    {
        return action( data->conn, data->stmts );
    }
    catch ( Exception& )
    {
        if ( data->conn.isDead() )
        {
            lease.reset();
        }

        throw;
    }
}

}

}
