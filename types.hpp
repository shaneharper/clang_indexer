#pragma once

#include <map>
#include <set>
#include <string>


typedef std::map< std::string /*referenced USR*/, std::set<std::string /*location*/> > ClicIndex;
typedef std::pair< std::string /*referenced USR*/, std::set<std::string /*location*/> > ClicIndexItem;
