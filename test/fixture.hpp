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

#include "connection.hpp"
#include "table.ipp"

struct Fixture
{
    rodbc::Environment env;
    rodbc::Connection conn{ env, RODBC_TEST_CONN_STR };
};

template< typename Column >
struct CreateSimpleTable : rodbc::CreateTable< std::tuple< Column > >
{
    CreateSimpleTable( rodbc::Connection& conn )
    : rodbc::CreateTable< std::tuple< Column > >{ conn, "tbl", { "col" }, rodbc::DROP_TABLE_IF_EXISTS | rodbc::TEMPORARY_TABLE }
    {
    }
};
