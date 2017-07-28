#pragma once

#include <boost/noncopyable.hpp>

namespace rodbc
{

class Connection : private boost::noncopyable
{
public:
    Connection( const char* const connStr );
    ~Connection();

    bool isDead() const;

private:
    void* env_;
    void* dbc_;

    friend class Transaction;
    friend class Statement;
};

class Transaction : private boost::noncopyable
{
public:
    Transaction( Connection& conn );
    ~Transaction();

    Transaction( Transaction&& );
    Transaction& operator=( Transaction&& );

    void commit();

private:
    void* dbc_;
};

}
