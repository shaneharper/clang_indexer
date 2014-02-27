#pragma once

#include <map>
#include <set>
#include <sstream>
#include <string>


struct Location
{
    static std::string locationString(const char* pathname, unsigned line, unsigned column, unsigned /*CXCursorKind*/ kind)
    {
        std::stringstream ss;
        ss << pathname << ":" << line << ":" << column << ":" << kind;
        return ss.str();
    }
};

typedef std::map< std::string /*referenced USR*/, std::set<std::string /*XXX Location*/> > ClicIndex;
typedef std::pair< std::string /*referenced USR*/, std::set<std::string /*XXX Location*/> > ClicIndexItem;
