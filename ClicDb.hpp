#pragma once

#include <set>
#include <string>


class ClicDb {
    public:
        ClicDb(const char* dbFilename);

        ~ClicDb();

        void clear();

        void set(const std::string& usr, const std::set<std::string>& locations);

        std::set<std::string> get(const std::string& usr);

        void addMultiple(const std::string& usr, const std::set<std::string>& locationsToAdd);

        void rmMultiple(const std::string& usr, const std::set<std::string>& locationsToRemove);

    private:
};

