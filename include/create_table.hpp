#pragma once

#include <types.hpp>

#include <boost/mpl/for_each.hpp>

#include <vector>

namespace rodbc
{

class Connection;

constexpr unsigned DROP_TABLE_IF_EXISTS = 1 << 0;
constexpr unsigned TEMPORARY_TABLE = 1 << 1;
constexpr unsigned FIRST_COLUMN_IS_PRIMARY_KEY = 1 << 2;

/**
 * @brief The CreateTable struct template
 */
template< typename Columns >
struct CreateTable
{
    CreateTable( Connection& conn, const char* const tableName, const std::vector< const char* >& columnNames, const unsigned flags = 0 );
};

namespace detail
{

template< typename Type > struct ColumnType;
template<> struct ColumnType< std::int8_t > { static constexpr const char* value = "TINYINT"; };
template<> struct ColumnType< std::int16_t > { static constexpr const char* value = "SMALLINT"; };
template<> struct ColumnType< std::int32_t > { static constexpr const char* value = "INT"; };
template<> struct ColumnType< std::int64_t > { static constexpr const char* value = "BIGINT"; };
template<> struct ColumnType< std::uint8_t > { static constexpr const char* value = "TINYINT UNSIGNED"; };
template<> struct ColumnType< std::uint16_t > { static constexpr const char* value = "SMALLINT UNSIGNED"; };
template<> struct ColumnType< std::uint32_t > { static constexpr const char* value = "INT UNSIGNED"; };
template<> struct ColumnType< std::uint64_t > { static constexpr const char* value = "BIGINT UNSIGNED"; };
template<> struct ColumnType< float > { static constexpr const char* value = "REAL"; };
template<> struct ColumnType< double > { static constexpr const char* value = "DOUBLE PRECISION"; };
template< std::size_t Size > struct ColumnType< String< Size > > { static constexpr const char* value = "TEXT"; };
template<> struct ColumnType< Timestamp > { static constexpr const char* value = "TIMESTAMP"; };
template< typename Type > struct ColumnType< Nullable< Type > > { static constexpr const char* value = ColumnType< Type >::value; };

void dropTableIfExists( Connection& conn, const char* const name );
void createTable( Connection& conn, const char* const name, const char* const definition, const bool temporary );

void defineColumn( std::string& definition, const char* const name, const char* const type, const bool first, const bool firstIsPrimaryKey );

struct DefineColumns
{
    DefineColumns( std::string& definition, const std::vector< const char* >& names, const bool firstIsPrimaryKey )
    : definition_{ definition }
    , names_{ names }
    , firstIsPrimaryKey_{ firstIsPrimaryKey }
    {
    }

    template< typename Type >
    void operator() ( const Type& )
    {
        defineColumn( definition_, names_[ index_ ], ColumnType< Type >::value, index_ == 0, firstIsPrimaryKey_ );

        ++index_;
    }

private:
    std::string& definition_;
    const std::vector< const char* >& names_;
    const bool firstIsPrimaryKey_;

    std::size_t index_{ 0 };
};

}

template< typename Columns >
inline CreateTable< Columns >::CreateTable( Connection& conn, const char* const tableName, const std::vector< const char* >& columnNames, const unsigned flags )
{
    if ( flags & DROP_TABLE_IF_EXISTS )
    {
        detail::dropTableIfExists( conn, tableName );
    }

    std::string tableDefinition;
    boost::mpl::for_each< Columns >( detail::DefineColumns{ tableDefinition, columnNames, flags & FIRST_COLUMN_IS_PRIMARY_KEY } );

    detail::createTable( conn, tableName, tableDefinition.c_str(), flags & TEMPORARY_TABLE );
}

}
