#include "connection_pool.ipp"

namespace rodbc
{
namespace detail
{

ConnectionPoolBase::ConnectionPoolBase( std::string&& connStr )
: connStr_{ std::move( connStr ) }
{
}

ConnectionPoolBase::~ConnectionPoolBase() = default;

Connection ConnectionPoolBase::makeConnection()
{
    boost::unique_lock< boost::mutex > lock{ env_lock_ };

    return { env_, connStr_.c_str() };
}

}

}
