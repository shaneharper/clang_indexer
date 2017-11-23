#include "Reference.hpp"

#include <string.h>

// ----- STATICS -----

std::ostream& operator<<( std::ostream& os, const Reference& ref)
{
    os << ref.m_marker<<":"<< ref.m_location;
    return os;
}

// ----- PUBLIC -----

Reference::Reference( const char* marker, Location location)
    : m_marker(marker)
    , m_location(location)
{
    // Do nothing
}


const Location& Reference::getLocation() const
{
    return m_location;
}

bool Reference::operator< ( const Reference& other ) const
{
    if ( m_marker < other.m_marker )
    {
        return true;
    }
    return m_location < other.m_location;

}
