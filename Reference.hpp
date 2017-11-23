#pragma once

#include "Location.hpp"

class Reference
{
public:
    Reference( const char* marker, Location location);

    const Location& getLocation() const;

    bool operator< (const Reference& other) const;

    friend std::ostream& operator<< (std::ostream& os, const Reference& ref);

private:
    std::string m_marker;
    Location m_location;
};
