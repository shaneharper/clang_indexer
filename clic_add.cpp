#include "clic_printer.hpp"
#include "types.hpp"

extern "C" {
#include <clang-c/Index.h>
}
#include "ClicDb.hpp"
#include <boost/foreach.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

// This code intentionally leaks memory like a sieve because the program is shortlived.

class IVisitor {
public:
    virtual enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent) = 0;
};

class EverythingIndexer : public IVisitor {
public:
    EverythingIndexer(const char* translationUnitFilename)
        : translationUnitFilename(translationUnitFilename) {}

    virtual enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent) {
        CXFile file;
        unsigned int line, column, offset;
        clang_getInstantiationLocation(
                clang_getCursorLocation(cursor),
                &file, &line, &column, &offset);
        CXCursorKind kind = clang_getCursorKind(cursor);
        const char* cursorFilename = clang_getCString(clang_getFileName(file));

        if (!clang_getFileName(file).data || strcmp(cursorFilename, translationUnitFilename) != 0) {
            return CXChildVisit_Continue;
        }

        CXCursor refCursor = clang_getCursorReferenced(cursor);
        if (!clang_equalCursors(refCursor, clang_getNullCursor())) {
            CXFile refFile;
            unsigned int refLine, refColumn, refOffset;
            clang_getInstantiationLocation(
                    clang_getCursorLocation(refCursor),
                    &refFile, &refLine, &refColumn, &refOffset);

            if (clang_getFileName(refFile).data) {
                std::string referencedUsr(clang_getCString(clang_getCursorUSR(refCursor)));
                if (!referencedUsr.empty()) {
                    std::stringstream ss;
                    ss << cursorFilename
                       << ":" << line << ":" << column << ":" << kind;
                    std::string location(ss.str());
                    usrToReferences[referencedUsr].insert(location);
                }
            }
        }
        return CXChildVisit_Recurse;
    }

    const char* translationUnitFilename;
    ClicIndex usrToReferences;
};

enum CXChildVisitResult visitorFunction(
        CXCursor cursor,
        CXCursor parent,
        CXClientData clientData)
{
    IVisitor* visitor = (IVisitor*)clientData;
    return visitor->visit(cursor, parent);
}

static bool has_errors(const CXTranslationUnit& tu) {
    for (unsigned i = 0; i != clang_getNumDiagnostics(tu); ++i) {
        const CXDiagnosticSeverity diagnostic_severity =
            clang_getDiagnosticSeverity(clang_getDiagnostic(tu, i));
        if (CXDiagnostic_Error == diagnostic_severity
            || CXDiagnostic_Fatal == diagnostic_severity) {
            return true;
        }
    }
    return false;
}


int main(int argc, const char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage:\n"
            << "    " << argv[0] << " <dbFilename> <indexFilename> [<options>] <sourceFilename>\n";
        return 1;
    }

    const char* const dbFilename = argv[1];
    const char* const indexFilename = argv[2];
    const char* const sourceFilename = argv[argc-1];

    // Set up the clang translation unit
    CXIndex cxindex = clang_createIndex(0, /*displayDiagnostics*/ 1);
    CXTranslationUnit tu = clang_parseTranslationUnit(
        cxindex, NULL,
        argv + 3, argc - 3, // Skip over program name (argv[0]), dbFilename and indexFilename
        0, 0,
        CXTranslationUnit_None);
    if (tu == NULL) {
        std::cerr << "clang_parseTranslationUnit failed." << std::endl;
        return 1;
    }

    if (has_errors(tu)) {
        return 1;
    }

    // Create the index
    EverythingIndexer visitor(sourceFilename);
    clang_visitChildren(
            clang_getTranslationUnitCursor(tu),
            &visitorFunction,
            &visitor);
    ClicIndex& index = visitor.usrToReferences;

    // OK, now write the index to a compressed file
    std::ofstream fout(indexFilename);
    boost::iostreams::filtering_stream<boost::iostreams::output> zout;
    zout.push(boost::iostreams::gzip_compressor());
    zout.push(fout);
    printIndex(zout, index);

    // Now open the database and add the index to it
    ClicDb db(dbFilename);

    BOOST_FOREACH(const ClicIndex::value_type& it, index) {
        db.addMultiple(/*usr*/ it.first, it.second);
    }

    return 0;
}
