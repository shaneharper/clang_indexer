#ifndef CLANG_INDEXER_TYPES_HPP__
#define CLANG_INDEXER_TYPES_HPP__

#include <string>
#include <map>
#include <set>

typedef std::map< std::string, std::set<std::string> > ClicIndex;
typedef std::pair< std::string, std::set<std::string> > ClicIndexItem;

#endif // CLANG_INDEXER_TYPES_HPP__
