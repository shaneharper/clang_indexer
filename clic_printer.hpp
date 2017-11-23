#ifndef CLANG_INDEXER_CLIC_PRINTER_HPP__
#define CLANG_INDEXER_CLIC_PRINTER_HPP__

#include "types.hpp"

#include <iosfwd>

void printIndex(
    std::ostream& out,
    const ClicIndex&);

void printLocations(
    std::ostream& out,
    const std::set<std::string>& locations);

#endif // CLANG_INDEXER_CLIC_PRINTER_HPP__
