#include "Reference.hpp"

#include <string.h>

#include <iostream>

// ----- STATICS -----

std::ostream& operator<<( std::ostream& os, const Reference& ref)
{
    os << ref.m_marker;
    return os;
}

// ----- PUBLIC -----

Reference::Reference( std::string marker)
    : m_marker(marker)
{
    // Do nothing
}

std::string Reference::toSafeCommaString() const
{
    std::stringstream ss;
    ss << "'"<<m_marker<<"'";
    return ss.str();
}

bool Reference::operator< ( const Reference& other ) const
{
    return m_marker < other.m_marker;
}

