
extern "C" {
#include <clang-c/Index.h>
}
#include <fstream>
#include <string.h>
#include <iostream>
#include <cassert>

#include "clic_printer.hpp"
#include "types.hpp"
#include "Location.hpp"
#include "Reference.hpp"
#include "ClicDb.hpp"
#include "CXStringWrapper.hpp"

class IVisitor 
{
public:
    virtual CXChildVisitResult visit(CXCursor cursor, CXCursor parent) = 0;
};

class EverythingIndexer : 
    public IVisitor 
{
public:
    EverythingIndexer(const char* nameOfFileToIndex)
        : m_nameOfFileToIndex(nameOfFileToIndex) 
    {
    }

    virtual CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/) 
    {
        CXFile file;
        unsigned line, column;
        clang_getExpansionLocation(
            clang_getCursorLocation(cursor),
            &file, &line, &column, nullptr);

        std::string cursorFilename = CXStringWrapper(clang_getFileName(file));
        if (cursorFilename.empty() || cursorFilename != m_nameOfFileToIndex ) 
        {
            return CXChildVisit_Continue;
            // XXX Rather the ignore the file, we could index it now (if not already indexed) - perhaps the file is an include file we wish to index.
        }



        CXCursorKind kind = clang_getCursorKind(cursor);

        std::string marker = CXStringWrapper(clang_getCursorUSR(cursor));
        if ( marker.empty() && clang_isReference( kind ) )
        {
            CXCursor refCursor = clang_getCursorReferenced(cursor);
            assert ( !clang_equalCursors(refCursor, clang_getNullCursor()));

            marker = CXStringWrapper(clang_getCursorUSR(refCursor));
        }
        if ( marker.empty() == false )
        {
            CXSourceRange range = clang_Cursor_getSpellingNameRange(cursor, 0, 0);
            CXFile startFile;
            unsigned startLine, startColumn;
            clang_getExpansionLocation(
                clang_getRangeStart(range),
                &startFile, &startLine, &startColumn, nullptr);
            CXFile endFile;
            unsigned endLine, endColumn;
            clang_getExpansionLocation(
                clang_getRangeEnd(range),
                &endFile, &endLine, &endColumn, nullptr);

            Reference ref(marker);
            Location startLoc(CXStringWrapper(clang_getFileName(startFile)), startLine, startColumn, kind);
            Location endLoc(CXStringWrapper(clang_getFileName(endFile)), endLine, endColumn, kind);
            if ( USR_ToReferences.count( ref) == 0 )
            {
                ClicIndex::value_type value( ref, {startLoc});
                USR_ToReferences.insert(value);
            }
            else
            {
                USR_ToReferences[ref].insert({endLoc});
            }
        }

        return CXChildVisit_Recurse;
    }

    ClicIndex USR_ToReferences;
private:
    std::string m_nameOfFileToIndex;
};

CXChildVisitResult visitorFunction(
    CXCursor cursor,
    CXCursor parent,
    CXClientData clientData)
{
    IVisitor* visitor = (IVisitor*)clientData;
    return visitor->visit(cursor, parent);
}

static void output_diagnostics(const CXTranslationUnit& tu) 
{
    for (unsigned i = 0; i != clang_getNumDiagnostics(tu); ++i) 
    {
        std::cerr<< (std::string)CXStringWrapper(
            clang_formatDiagnostic(
                clang_getDiagnostic(tu, i),
                clang_defaultDiagnosticDisplayOptions()
                )
            )
            << std::endl;
    }
}

static bool has_errors(const CXTranslationUnit& tu) 
{
    for (unsigned i = 0; i != clang_getNumDiagnostics(tu); ++i) 
    {
        const CXDiagnosticSeverity diagnostic_severity =
            clang_getDiagnosticSeverity(clang_getDiagnostic(tu, i));
        if (CXDiagnostic_Error == diagnostic_severity ||
            CXDiagnostic_Fatal == diagnostic_severity) 
        {
            return true;
        }
    }
    return false;
}



void addToDb(const char* dbFilename, const ClicIndex& index)
{
    ClicDb db(dbFilename);

    for(const ClicIndex::value_type& it: index) {
        db.addMultiple(/*USR*/ it.first, it.second);
    }
}


int main(int argc, const char* argv[]) {
    if (argc < 4) 
    {
        std::cerr << "Usage:\n"
            << "    " << argv[0] << " <dbFilename> <sourceFilename> [<options>] \n";
        return 1;
    }
    const char* const dbFilename = argv[1];
    std::cout<<"Database file name: "<<dbFilename<<std::endl;
    const char* const sourceFilename = argv[2];
    std::cout<<"Source file name: "<<sourceFilename<<std::endl;

    CXIndex cxindex = clang_createIndex(0, /*displayDiagnostics*/ 1);
    CXTranslationUnit tu;
    CXErrorCode error = clang_parseTranslationUnit2(
        cxindex, 
        sourceFilename,
        argv + 3, 
        argc - 3, // Skip over program name (argv[0]), dbFilename and indexFilename
        nullptr, 
        0,
        CXTranslationUnit_None,
        &tu);
    if (error != CXError_Success ) 
    {
        std::cerr << "clang_parseTranslationUnit failed: "<< error << std::endl;
        return 1;
    }
    output_diagnostics(tu);
    if (has_errors(tu)) 
    {
        return 1;
    }

    // Create the index
    EverythingIndexer visitor(sourceFilename);
    clang_visitChildren(
        clang_getTranslationUnitCursor(tu),
        &visitorFunction,
        &visitor);

    addToDb(dbFilename, visitor.USR_ToReferences);

    return 0;
}
