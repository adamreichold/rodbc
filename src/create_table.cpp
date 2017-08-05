#include "create_table.hpp"

#include "statement.hpp"

namespace rodbc
{
namespace detail
{

void dropTableIfExists( Connection& conn, const char* const name )
{
    std::string stmt{ "DROP TABLE IF EXISTS " };

    stmt += name;
    stmt += ';';

    Statement{ conn, stmt.c_str() }.exec();
}

void createTable( Connection& conn, const char* const name, const char* const definition, const bool temporary )
{
    std::string stmt{ "CREATE " };

    if ( temporary )
    {
        stmt += "TEMPORARY ";
    }

    stmt += "TABLE ";
    stmt += name;
    stmt += " (";
    stmt += definition;
    stmt += ");";

    Statement{ conn, stmt.c_str() }.exec();
}

void defineColumn( std::string& definition, const char* const name, const char* const type, const bool first, const bool firstIsPrimaryKey )
{
    if ( !first )
    {
        definition += ", ";
    }

    definition += name;
    definition += ' ';
    definition += type;

    if ( first && firstIsPrimaryKey )
    {
        definition += " PRIMARY KEY";
    }
}

}
}
