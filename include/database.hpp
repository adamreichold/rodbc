#pragma once

#include "connection.hpp"

#include <boost/noncopyable.hpp>
#include <boost/thread/tss.hpp>

namespace rodbc
{

template< typename Statements >
class Database : private boost::noncopyable
{
public:
    Database( const char* const connStr );

protected:
    class BoundTransaction : private boost::noncopyable
    {
    protected:
        BoundTransaction( Database& database );
        BoundTransaction( BoundTransaction&& );

        void doCommit();

    private:
        Database& database_;
        Transaction transaction_;
    };

    class BoundStatement : private boost::noncopyable
    {
    protected:
        BoundStatement( Database& database );

        Database& database_;
        Statements& stmts_;

        template< typename Statement >
        void doExec( Statement& stmt );
        template< typename Statement >
        bool doFetch( Statement& stmt );
    };

protected:
    struct Session
    {
        Session( const char* const connStr );

        Connection conn;
        Statements stmts;
    };

    Session& openSession();

    template< typename Action >
    auto closeDeadSession( Action action ) -> decltype( action() );

private:
    const char* const connStr_;
    boost::thread_specific_ptr< Session > session_;
};

}
