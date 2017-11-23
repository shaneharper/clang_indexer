#include "ClicDb.hpp"

#include "clic_printer.hpp"
#include <iostream>

#define SQLITE_EXEC( command )\
    { \
        char* errors;\
        int error = sqlite3_exec(m_pDb, command,  &ClicDb::sqliteCallback, this, &errors);\
        if ( error != SQLITE_OK ) \
        {\
            std::cerr<<"Error: "<<error<<": "<<errors<<std::endl;\
        }\
    }

int ClicDb::sqliteCallback( void* pClickDb, int nColumns, char** pColumns, char** columnNames)
{
    for(int i =0 ; i<nColumns; i++)
    {
        std::cout<<pColumns[i];
    }
    std::cout<<std::endl;
    return 0;
}

ClicDb::ClicDb(const char* dbFilename)
{
    std::cout<<"-- Database "<<dbFilename<<" --"<<std::endl;
    int error = sqlite3_open(dbFilename, &m_pDb);
    if ( error == SQLITE_OK && m_pDb != nullptr )
    {
        // Success
        std::cout<<"Success "<<std::endl;
    }
    else if ( m_pDb != nullptr )
    {
        std::cerr<<"Error opening database: "<<error<<": "<<sqlite3_errmsg(m_pDb)<<std::endl;
    }
    else 
    {
        std::cerr<<"Error opening database: "<<error<<": "<<std::endl;
    }

    SQLITE_EXEC("SELECT name FROM sqlite_master WHERE type='table'");
    SQLITE_EXEC("CREATE TABLE IF NOT EXISTS Reference( id INTEGER PRIMARY KEY AUTOINCREMENT, marker TEXT UNIQUE, INTEGER location)");
    SQLITE_EXEC("CREATE TABLE IF NOT EXISTS Location( id INTEGER PRIMARY KEY AUTOINCREMENT, filename TEXT UNIQUE , line INTEGER UNIQUE, column INTEGER UNIQUE, kind INTEGER UNIQUE)");
    SQLITE_EXEC("SELECT name FROM sqlite_master WHERE type='table'");
}

ClicDb::~ClicDb() 
{
    std::cout<<"~~ Database "<<" ~~"<<std::endl;
    sqlite3_close(m_pDb);
}

void ClicDb::clear() 
{
}

void ClicDb::add(const Reference& usr, const std::set<Location>& locations) 
{

}

std::set<Location> ClicDb::get(const Reference& ref) 
{
    std::set<Location> result;
    return result;
}

void ClicDb::addMultiple(const Reference& ref, const std::set<Location>& locationsToAdd) 
{
    std::cout<<"Adding: "<<ref<<std::endl;
    for ( const Location& location : locationsToAdd )
    {
        std::cout<<"-->: "<<location<<std::endl;
        std::string command =  "INSERT INTO Location (";
        command += Location::locationCommaString(location);
        command += ")";
        SQLITE_EXEC(command.c_str());
    }
    SQLITE_EXEC("SELECT * FROM Location");
}

