#pragma once
#include "connection_pool.hpp"

#include "types.hpp"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/tss.hpp>

#include <vector>

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

    boost::thread_specific_ptr< Data > data_;

protected:
    class LeaseImpl
    {
    private:
        Impl& impl_;

    protected:
        LeaseImpl( Impl& impl )
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
};

template< typename Statements >
class FixedSizeConnectionPoolImpl
{
private:
    using Impl = FixedSizeConnectionPoolImpl;
    using Data = ConnectionPoolData< Statements >;

    boost::mutex data_lock_;
    boost::condition_variable data_condition_;
    std::vector< std::unique_ptr< Data > > data_;

protected:
    class LeaseImpl
    {
    private:
        Impl& impl_;
        std::unique_ptr< Data > data_;

    protected:
        LeaseImpl( Impl& impl )
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

        ~LeaseImpl()
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
inline ConnectionPool< Statements, ConnectionPoolImpl >::Lease::Lease( ConnectionPool& pool )
: ConnectionPoolImpl::LeaseImpl{ pool }
, pool_{ pool }
{
}

template< typename Statements, typename ConnectionPoolImpl >
template< typename Action >
inline auto ConnectionPool< Statements, ConnectionPoolImpl >::Lease::operator() ( Action action ) -> typename std::result_of< Action( Connection&, Statements& ) >::type
{
    using Impl = typename ConnectionPoolImpl::LeaseImpl;
    using Data = ConnectionPoolData< Statements >;

    auto* data = Impl::get();

    if ( !data )
    {
        data = new Data{ pool_.makeConnection() };

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

}

}
