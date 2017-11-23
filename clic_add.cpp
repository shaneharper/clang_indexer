#include "clic_printer.hpp"
#include "types.hpp"

extern "C" {
#include <clang-c/Index.h>
}
#include "ClicDb.hpp"
#include <fstream>
#include <string.h>
#include <iostream>

// This code leaks memory like a sieve, but it's short-lived

class IVisitor {
public:
    virtual CXChildVisitResult visit(CXCursor cursor, CXCursor parent) = 0;
};

class EverythingIndexer : 
    public IVisitor 
{
    const char* nameOfFileToIndex;
public:
    EverythingIndexer(const char* nameOfFileToIndex)
        : nameOfFileToIndex(nameOfFileToIndex) 
    {
    }

    virtual CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/) 
    {
        CXFile file;
        unsigned line, column, offset;
        clang_getExpansionLocation(
            clang_getCursorLocation(cursor),
            &file, &line, &column, &offset);
        const char* cursorFilename = clang_getCString(clang_getFileName(file));

        if (!clang_getFileName(file).data || strcmp(cursorFilename, nameOfFileToIndex) != 0) 
        {
            return CXChildVisit_Continue;
            // XXX Rather the ignore the file, we could index it now (if not already indexed) - perhaps the file is an include file we wish to index.
        }

        CXCursor refCursor = clang_getCursorReferenced(cursor);
        if (!clang_equalCursors(refCursor, clang_getNullCursor())) 
        {
            CXFile refFile;
            clang_getExpansionLocation(
                clang_getCursorLocation(refCursor),
                &refFile, nullptr, nullptr, nullptr);

            if (clang_getFileName(refFile).data) 
            {
                const std::string referencedUSR(clang_getCString(clang_getCursorUSR(refCursor)));
                if (!referencedUSR.empty()) 
                {
                    unsigned refKind = clang_getCursorKind(refCursor);
                    USR_ToReferences[referencedUSR].insert(Location::locationString(cursorFilename, line, column, refKind));
                }
            }
        }
        return CXChildVisit_Recurse;
    }

    ClicIndex USR_ToReferences;
};

CXChildVisitResult visitorFunction(
    CXCursor cursor,
    CXCursor parent,
    CXClientData clientData)
{
    IVisitor* visitor = (IVisitor*)clientData;
    return visitor->visit(cursor, parent);
}

static void output_diagnostics(const CXTranslationUnit& tu) {
    for (unsigned i = 0; i != clang_getNumDiagnostics(tu); ++i) {
        std::cerr
            << clang_getCString(
                clang_formatDiagnostic(
                    clang_getDiagnostic(tu, i),
                    clang_defaultDiagnosticDisplayOptions()))
            << std::endl;
    }
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


void write(const char* indexFilename, const ClicIndex& index)
{
    std::ofstream fout(indexFilename);
    printIndex(fout, index);
}


void addToDb(const char* dbFilename, const ClicIndex& index)
{
    ClicDb db(dbFilename);

    for(const ClicIndex::value_type& it: index) {
        db.addMultiple(/*USR*/ it.first, it.second);
    }
}


int main(int argc, const char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage:\n"
            << "    " << argv[0] << " <dbFilename> <indexFilename> <sourceFilename> [<options>] \n";
        return 1;
    }
    const char* const dbFilename = argv[1];
    const char* const indexFilename = argv[2];
    const char* const sourceFilename = argv[3];
    std::cout<<"Source file name: "<<sourceFilename<<std::endl;

    CXIndex cxindex = clang_createIndex(0, /*displayDiagnostics*/ 1);
    CXTranslationUnit tu;
    CXErrorCode error = clang_parseTranslationUnit2(
        cxindex, 
        sourceFilename,
        argv + 4, 
        argc - 4, // Skip over program name (argv[0]), dbFilename and indexFilename
        nullptr, 
        0,
        CXTranslationUnit_None,
        &tu);
    if (error != CXError_Success ) {
        std::cerr << "clang_parseTranslationUnit failed: "<< error << std::endl;
        return 1;
    }
    output_diagnostics(tu);
    if (has_errors(tu)) {
        return 1;
    }

    // Create the index
    EverythingIndexer visitor(sourceFilename);
    clang_visitChildren(
        clang_getTranslationUnitCursor(tu),
        &visitorFunction,
        &visitor);

    write(indexFilename, visitor.USR_ToReferences); // Note: the index files and the database contain the same information. Note that the db isn't keyed on source filename (the index files are though)- finding all references from a particular source file (only) occurs when updating the db after deleting a source file.
    addToDb(dbFilename, visitor.USR_ToReferences);

    return 0;
}
