#pragma once

#include "Location.hpp"

class Reference
{
public:
    Reference( std::string marker);

    std::string toSafeCommaString() const;

    bool operator< (const Reference& other) const;

    friend std::ostream& operator<< (std::ostream& os, const Reference& ref);

private:
    std::string m_marker;
};
