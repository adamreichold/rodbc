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
#include "connection_pool.ipp"

#include <boost/thread/lock_types.hpp>

namespace rodbc
{
namespace detail
{

ConnectionPoolBase::ConnectionPoolBase( std::string connStr )
: connStr_{ std::move( connStr ) }
{
}

ConnectionPoolBase::~ConnectionPoolBase() = default;

Connection ConnectionPoolBase::makeConnection()
{
    boost::unique_lock< boost::mutex > lock{ env_lock_ };

    return { env_, connStr_.c_str() };
}

ConnectionPoolHolderBase::~ConnectionPoolHolderBase() = default;

ThreadLocalConnectionPoolImpl::ThreadLocalConnectionPoolImpl() = default;

ThreadLocalConnectionPoolImpl::~ThreadLocalConnectionPoolImpl() = default;

ThreadLocalConnectionPoolImpl::LeaseImpl::LeaseImpl( ThreadLocalConnectionPoolImpl& impl )
: impl_{ impl }
{
}

ConnectionPoolHolderBase* ThreadLocalConnectionPoolImpl::LeaseImpl::get() const
{
    return impl_.holder_.get();
}

void ThreadLocalConnectionPoolImpl::LeaseImpl::reset( ConnectionPoolHolderBase* const holder )
{
    impl_.holder_.reset( holder );
}

FixedSizeConnectionPoolImpl::FixedSizeConnectionPoolImpl( const std::size_t size )
: holder_{ size }
{
}

FixedSizeConnectionPoolImpl::~FixedSizeConnectionPoolImpl() = default;

FixedSizeConnectionPoolImpl::LeaseImpl::LeaseImpl( FixedSizeConnectionPoolImpl& impl )
: impl_{ impl }
{
    boost::unique_lock< boost::mutex > lock{ impl_.holder_lock_ };

    while ( impl_.holder_.empty() )
    {
        impl_.holder_condition_.wait( lock );
    }

    holder_ = std::move( impl_.holder_.back() );

    impl_.holder_.pop_back();
}

FixedSizeConnectionPoolImpl::LeaseImpl::~LeaseImpl()
{
    boost::unique_lock< boost::mutex > lock{ impl_.holder_lock_ };

    impl_.holder_.push_back( std::move( holder_ ) );

    impl_.holder_condition_.notify_one();
}

ConnectionPoolHolderBase* FixedSizeConnectionPoolImpl::LeaseImpl::get() const
{
    return holder_.get();
}

void FixedSizeConnectionPoolImpl::LeaseImpl::reset( ConnectionPoolHolderBase* const holder )
{
    holder_.reset( holder );
}

}
}
