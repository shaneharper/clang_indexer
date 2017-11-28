#include "ClicDb.hpp"

#include "clic_printer.hpp"
#include <iostream>

#define SQLITE_EXEC( command )\
    { \
        char* errors;\
        int error = sqlite3_exec(m_pDb, command,  &ClicDb::sqliteCallback, this, &errors);\
        if ( error != SQLITE_OK ) \
        {\
            std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<": "<<errors<<std::endl;\
            std::cerr<<command<<std::endl;\
        }\
    }

int ClicDb::sqliteCallback( void* pClickDb, int nColumns, char** pColumns, char** columnNames)
{
    for(int i =0 ; i<nColumns; i++)
    {
        std::cout<<pColumns[i]<<", ";
    }
    std::cout<<std::endl;
    return 0;
}

ClicDb::ClicDb(const char* dbFilename)
{
    int error = sqlite3_open(dbFilename, &m_pDb);
    if ( error == SQLITE_OK && m_pDb != nullptr )
    {
        // Success
    }
    else if ( m_pDb != nullptr )
    {
        std::cerr<<"Error opening database: "<<error<<": "<<sqlite3_errmsg(m_pDb)<<std::endl;
    }
    else 
    {
        std::cerr<<"Error opening database: "<<error<<": "<<std::endl;
    }

    SQLITE_EXEC("CREATE TABLE IF NOT EXISTS Reference( id INTEGER PRIMARY KEY AUTOINCREMENT, marker TEXT UNIQUE)");
    SQLITE_EXEC("CREATE TABLE IF NOT EXISTS Location( id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "filename TEXT NOT NULL, "
                "line INTEGER NOT NULL, "
                "column INTEGER NOT NULL, "
                "kind INTEGER NOT NULL, "
                "reference INTEGER NOT NULL, "
                "UNIQUE ( filename, line, column, kind, reference) )");
}

ClicDb::~ClicDb() 
{
    sqlite3_close(m_pDb);
}

void ClicDb::clear() 
{
}

std::set<Location> ClicDb::get(const Reference& ref) 
{

    int id = findIdForReference( ref );

    std::string where = "reference IS ";
    where += "'";
    where += std::to_string(id);
    where += "'";

    return findLocationsWhere( where );
}

void ClicDb::addMultiple(const Reference& ref, const std::set<Location>& locationsToAdd) 
{
    std::string command =  "INSERT OR IGNORE INTO Reference ( marker) VALUES (";
    command += ref.toSafeCommaString();
    command += ")";

    SQLITE_EXEC( command.c_str() );

    int id = findIdForReference( ref );

    std::cout<<"Adding: "<<ref<<": "<<id<<std::endl;

    command =  "INSERT OR IGNORE INTO Location ( filename, line, column, kind, reference) VALUES ";
    auto i = locationsToAdd.begin();
    while ( i != locationsToAdd.end() )
    {
        std::cout<<"-->: "<<*i<<std::endl;
        command += "(";
        command += i->toSafeCommaString();
        command += ", '";
        command += std::to_string(id);
        command += "'";
        command += ")";
        ++i;
        if ( i != locationsToAdd.end() )
        {
            command += ", ";
        }
    }
    SQLITE_EXEC(command.c_str());
}

// ----- PRIVATE -----

int ClicDb::findIdForReference(const Reference& ref)
{
    std::string command = "SELECT id FROM Reference WHERE marker IS ";
    command += ref.toSafeCommaString();

    sqlite3_stmt* pStmt;
    int error = sqlite3_prepare_v2( m_pDb, command.c_str(), command.length() * sizeof( std::string::value_type), &pStmt, nullptr);
    if ( error != SQLITE_OK )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<command<<std::endl;
    }
    error = sqlite3_step( pStmt );
    if ( error != SQLITE_ROW )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<sqlite3_errstr(error)<<std::endl;
    }
    int id = sqlite3_column_int( pStmt, 0);
    error = sqlite3_step( pStmt );
    if ( error != SQLITE_DONE )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<sqlite3_errstr(error)<<std::endl;
    }
    error = sqlite3_finalize( pStmt );
    if ( error != SQLITE_OK )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<sqlite3_errstr(error)<<std::endl;
    }
    return id;
}

std::set<Location> ClicDb::findLocationsWhere( std::string where)
{
    std::set<Location> result;

    std::string command = "SELECT filename, line, column, kind FROM Location WHERE ";
    command += where;

    sqlite3_stmt* pStmt;
    int error = sqlite3_prepare_v2( m_pDb, command.c_str(), command.length() * sizeof( std::string::value_type), &pStmt, nullptr);
    if ( error != SQLITE_OK )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<command<<std::endl;
    }
    error = sqlite3_step( pStmt );
    while ( error == SQLITE_ROW )
    {
        std::string filename (reinterpret_cast<const char*>(sqlite3_column_text( pStmt, 0)));
        unsigned line = sqlite3_column_int( pStmt, 1);
        unsigned column = sqlite3_column_int( pStmt, 2);
        unsigned kind = sqlite3_column_int( pStmt, 3);
        result.insert( { filename, line, column, kind } );

        error = sqlite3_step( pStmt );
    }
    if ( error != SQLITE_DONE )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<sqlite3_errstr(error)<<std::endl;
    }
    error = sqlite3_finalize( pStmt );
    if ( error != SQLITE_OK )
    {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Error: "<<error<<std::endl;
        std::cerr<<sqlite3_errstr(error)<<std::endl;
    }

    return result;
}
