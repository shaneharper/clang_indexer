#pragma once

#include <sstream>
#include <string>

class Location
{
public:
    Location(const char* pathname, unsigned line, unsigned column, unsigned kind);

    static std::string locationString(const Location& location);
    static std::string locationString(const char* pathname, unsigned line, unsigned column, unsigned kind);
    static std::string locationCommaString(const Location& location);

    bool operator< (const Location& other) const;

    friend std::ostream& operator<< (std::ostream& os, const Location& location);

private:
    std::string m_pathname;
    unsigned m_line;
    unsigned m_column;
    unsigned m_kind;
};
