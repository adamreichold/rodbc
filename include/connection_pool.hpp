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
    boost::mutex env_lock_;
    Environment env_;

    const std::string connStr_;
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

    template< typename Action >
    auto operator() ( Action action ) -> typename std::result_of< Action(Connection&, Statements&) >::type;
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
