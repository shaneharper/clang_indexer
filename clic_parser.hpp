#pragma once

#include "types.hpp"

#include <istream>
#include <memory>

class IndexItemIterator  
{
public:

    IndexItemIterator();

    IndexItemIterator(std::istream& in);

private:

    const ClicIndexItem& dereference() const;
    bool equal(const IndexItemIterator& other) const;
    void increment();

    std::shared_ptr<ClicIndexItem> value;
    std::ptrdiff_t i;
    std::istream* in;
};

ClicIndex::iterator parseIndex(std::istream& in);

