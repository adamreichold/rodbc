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
#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <vector>

namespace rodbc
{
namespace detail
{

struct StmtExec
{
    template< typename Stmt >
    StmtExec( Stmt& stmt );
};

struct StmtFetch
{
};

template< typename Cols >
struct StmtIteratorBase : boost::intrusive_ref_counter< StmtIteratorBase< Cols >, boost::thread_unsafe_counter >
{
    virtual ~StmtIteratorBase() = default;

    virtual bool increment() = 0;
    virtual const Cols& dereference() const = 0;
};

template< typename Stmt, typename Cols >
struct StmtIterator : StmtIteratorBase< Cols >
{
    StmtIterator( Stmt& stmt );

    bool increment() override;
    const Cols& dereference() const override;

private:
    Stmt& stmt_;
};

template< typename Stmt, typename Cols >
struct StmtIterator< Stmt, std::vector< Cols > > : StmtIteratorBase< Cols >
{
    StmtIterator( Stmt& stmt );

    bool increment() override;
    const Cols& dereference() const override;

private:
    Stmt& stmt_;
    typename std::vector< Cols >::const_iterator row_;
};

}

/**
 * @brief The ResultSetIterator class template
 */
template< typename Cols >
class ResultSetIterator : public boost::iterator_facade< ResultSetIterator< Cols >, const Cols, std::input_iterator_tag >
{
private:
    template< typename Cols_ > friend class ResultSet;

    ResultSetIterator() = default;

    template< typename Stmt >
    ResultSetIterator( const detail::StmtFetch&, Stmt& stmt );

private:
    friend class boost::iterator_core_access;

    void increment();
    bool equal( const ResultSetIterator& other ) const;
    const Cols& dereference() const;

private:
    boost::intrusive_ptr< detail::StmtIteratorBase< Cols > > it_;
};

/**
 * @brief The ResultSet class template
 */
template< typename Cols >
class ResultSet : private detail::StmtExec
{
public:
    template< typename Stmt >
    ResultSet( Stmt& stmt );

    const ResultSetIterator< Cols >& begin() const;
    ResultSetIterator< Cols > end() const;

private:
    const ResultSetIterator< Cols > begin_;
};

}
