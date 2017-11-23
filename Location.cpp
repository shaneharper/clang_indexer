#include "Location.hpp"

#include <string.h>

// ----- STATICS -----

std::ostream& operator<<( std::ostream& os, const Location& location)
{
    os << Location::locationString( location);
    return os;
}

std::string Location::locationString(const Location& location)
{
    std::stringstream ss;
    ss << location.m_pathname << ":" << location.m_line << ":" << location.m_column << ":" << location.m_kind;
    return ss.str();
}

std::string Location::locationString(const char* pathname, unsigned line, unsigned column, unsigned /*CXCursorKind*/ kind)
{
    std::stringstream ss;
    ss << pathname << ":" << line << ":" << column << ":" << kind;
    return ss.str();
}

std::string Location::locationCommaString(const Location& location)
{
    std::stringstream ss;
    ss << location.m_pathname << ", " << location.m_line << ", " << location.m_column << ", " << location.m_kind;
    return ss.str();
}

// ----- PUBLIC -----

Location::Location( const char* pathname, unsigned line, unsigned column, unsigned kind)
    : m_pathname(pathname)
    , m_line(line)
    , m_column(column)
    , m_kind(kind)
{
    // Do nothing
}

bool Location::operator< ( const Location& other ) const
{
    if ( m_pathname < other.m_pathname )
    {
        return true;
    }
    if ( m_line < other.m_line )
    {
        return true;
    }
    if ( m_column < other.m_column )
    {
        return true;
    }
    return m_kind < other.m_kind;
}
