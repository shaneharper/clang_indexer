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

// This code intentionally leaks memory like a sieve because the program is shortlived.

class IVisitor {
public:
    virtual enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent) = 0;
};

class EverythingIndexer : public IVisitor {
public:
    virtual enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent) {
        CXCursor refCursor = clang_getCursorReferenced(cursor);
        if (!clang_equalCursors(refCursor, clang_getNullCursor())) {
            CXFile file;
            unsigned int line, column, offset;
            clang_getInstantiationLocation(
                    clang_getCursorLocation(cursor),
                    &file, &line, &column, &offset);
            CXCursorKind kind = clang_getCursorKind(cursor);

            CXFile refFile;
            unsigned int refLine, refColumn, refOffset;
            clang_getInstantiationLocation(
                    clang_getCursorLocation(refCursor),
                    &refFile, &refLine, &refColumn, &refOffset);

            if (clang_getFileName(file).data && clang_getFileName(refFile).data) {
                std::string referencedUsr(clang_getCString(clang_getCursorUSR(refCursor)));
                if (!referencedUsr.empty()) {
                    std::stringstream ss;
                    ss << clang_getCString(clang_getFileName(file))
                       << ":" << line << ":" << column << ":" << kind;
                    std::string location(ss.str());
                    usrToReferences[referencedUsr].insert(location);
                }
            }
        }
        return CXChildVisit_Recurse;
    }

    ClicIndex usrToReferences;
};

class DefinitionIndexer : public EverythingIndexer {
public:
    virtual enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent) {
        if (!clang_isCursorDefinition(cursor))
            return CXChildVisit_Recurse;
        return EverythingIndexer::visit(cursor, parent);
    }
};

enum CXChildVisitResult visitorFunction(
        CXCursor cursor,
        CXCursor parent,
        CXClientData clientData)
{
    IVisitor* visitor = (IVisitor*)clientData;
    return visitor->visit(cursor, parent);
}

int main(int argc, const char* argv[]) {
    assert(argc >= 3);
    const char* dbFilename = argv[1];
    const char* sourceFilename = argv[argc-1];
    argv++;
    argc--;

    // Set up the clang translation unit
    CXIndex cxindex = clang_createIndex(0, 0);
    CXTranslationUnit tu = clang_parseTranslationUnit(
        cxindex, 0,
        argv, argc,
        0, 0,
        CXTranslationUnit_None);

    // Print any errors or warnings
    int n = clang_getNumDiagnostics(tu);
    if (n > 0) {
        int nErrors = 0;
        for (unsigned i = 0; i != n; ++i) {
            CXDiagnostic diag = clang_getDiagnostic(tu, i);
            CXString string = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
            fprintf(stderr, "%s\n", clang_getCString(string));
            if (clang_getDiagnosticSeverity(diag) == CXDiagnostic_Error
                    || clang_getDiagnosticSeverity(diag) == CXDiagnostic_Fatal)
                nErrors++;
        }
    }

    // Create the index
    EverythingIndexer visitor;
    clang_visitChildren(
            clang_getTranslationUnitCursor(tu),
            &visitorFunction,
            &visitor);
    ClicIndex& index = visitor.usrToReferences;

    // OK, now write the index to a compressed file
    char* indexFilename = new char[strlen(sourceFilename)+10];
    sprintf(indexFilename, "%s.i.gz", sourceFilename);
    std::ofstream fout(indexFilename);
    boost::iostreams::filtering_stream<boost::iostreams::output> zout;
    zout.push(boost::iostreams::gzip_compressor());
    zout.push(fout);
    printIndex(zout, index);

    // Now open the database and add the index to it
    ClicDb db(dbFilename);

    BOOST_FOREACH(const ClicIndex::value_type& it, index) {
        const std::string& usr = it.first;
        db.addMultiple(usr, it.second);
    }

    return 0;
}