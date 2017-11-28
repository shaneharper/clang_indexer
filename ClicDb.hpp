#pragma once

#include <set>
#include <string>

#include "types.hpp"
#include "sqlite3.h"


class ClicDb {
    public:
        ClicDb(const char* dbFilename);

        ~ClicDb();

        void clear();

        std::set<Location> get(const Reference& ref);

        void addMultiple(const Reference& ref, const std::set<Location>& locationsToAdd); 

    private:

        int findIdForReference( const Reference& ref);
        std::set<Location> findLocationsWhere( std::string where);

        static int sqliteCallback(void* pClicDb, int nColumns, char** pColumns, char** columnNames);

        sqlite3* m_pDb;
};

