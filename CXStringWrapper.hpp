#pragma once

#include <clang-c/CXString.h>
#include <string>

class CXStringWrapper
{
public: 
    CXStringWrapper( CXString&& str)
        : m_str( str)
    {
        // Do nothing
    };

    ~CXStringWrapper()
    {
        clang_disposeString(m_str);
    };

    operator std::string() const
    {
        const char* value =  clang_getCString(m_str);
        if ( value )
        {
            return value;
        }
        else
        {
            return std::string();
        }
    };
private:
    CXString m_str;
};

