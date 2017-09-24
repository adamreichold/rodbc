Reasonable ODBC wrapper for C++
-------------------------------

This library provides a thin wrapper around the ODBC API isolating applications from the C headers and trying to improve ease of use for C++ programs. It aims to provide reasonable facilities without undue overhead. Since calls through the driver manager can be expensive, it also emphasizes bindings that stay valid for the whole lifetime of a statement and bulk operations that bind batches of parameters and rows.

It requires C++11 compiler support and depends on Boost, more specifically Fusion, Algorithm, DateTime, Format, Threads and Test. The build system is based on CMake including CTest and CPack.

The test suite uses a scratch database, an in-memory SQLite database by default, which can be configured using the CMake variable `RODBC_TEST_CONN_STR`. It has been successfully executed on Linux/amd64 using unixODBC 2.3 with SQLite 3.19, PostgreSQL 9.6 and MariaDB 10.1.

The API has three layers:
 * The `Environment`, `Connection`, `Transaction` and `Statement` classes are the lowest layer providing direct access to binding statements to arbitrary objects.
 * The `TypedStatement` and `StagedStatement` classes use Boost.Fusion to automatically handle the binding of adapted structures including sets of parameters and rows.
 * The `ConnectionPool` and `Database` classes use Boost.Thread to provide thread-local or shared connections with associated state and automatic detection of lost connections.

 For usage examples and further discussions, please refer to the unit tests and the inline documentation.
