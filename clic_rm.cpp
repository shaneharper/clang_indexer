#include "clic_parser.hpp"

#include "ClicDb.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage:\n"
            << "    " << argv[0] << " <dbFilename> <indexFilename>\n";
        return 1;
    }

    ClicDb db(argv[1]);

    std::ifstream file(argv[2]);
    if (file.fail()) {
        std::cerr << argv[2] << " cannot be opened" << std::endl;
    } else {
        // for(const ClicIndex::iterator& it: parseIndex(file)) {
        //     db.rmMultiple(it->first, it->second);
        // }
    }
}
