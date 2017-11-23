#include "ClicDb.hpp"

#include "clic_printer.hpp"
#include <iostream>

ClicDb::ClicDb(const char* dbFilename)
{
}

ClicDb::~ClicDb() 
{
}

void ClicDb::clear() 
{
}

void ClicDb::set(const std::string& usr, const std::set<std::string>& locations) 
{
}

std::set<std::string> ClicDb::get(const std::string& usr) 
{
    std::set<std::string> result;
    return result;
}

void ClicDb::addMultiple(const std::string& usr, const std::set<std::string>& locationsToAdd) 
{
    std::cout<<"Adding: "<<usr<<std::endl;
    for ( const std::string& location : locationsToAdd )
    {
        std::cout<<"-->: "<<location<<std::endl;
    }
}

void ClicDb::rmMultiple(const std::string& usr, const std::set<std::string>& locationsToRemove) 
{
}
