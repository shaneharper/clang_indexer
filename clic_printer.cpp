#include "clic_printer.hpp"

#include <ostream>

void printIndex(
    std::ostream& out,
    const ClicIndex& index)
{
    for(const ClicIndex::value_type& it: index) {
        out << it.first << '\t';
        printLocations(out, it.second);
        out << std::endl;
    }
}

void printLocations(std::ostream& out, const std::set<Location>& locations) {
    bool first = true;
    for(const Location& loc: locations) {
        out << loc;
        first = false;
    }
}
