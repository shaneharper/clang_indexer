#ifndef CLANG_INDEXER_TYPES_HPP__
#define CLANG_INDEXER_TYPES_HPP__

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

typedef std::map< std::string, std::set<std::string> > ClicIndex;
typedef std::pair< std::string, std::set<std::string> > ClicIndexItem;

#endif // CLANG_INDEXER_TYPES_HPP__

