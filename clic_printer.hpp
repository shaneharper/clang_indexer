#pragma once

#include "types.hpp"

#include <iosfwd>

void printIndex(
    std::ostream& out,
    const ClicIndex&);

void printLocations(
    std::ostream& out,
    const std::set<std::string>& locations);

