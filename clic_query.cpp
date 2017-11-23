#include "clic_printer.hpp"
#include "types.hpp"

extern "C" {
#include <clang-c/Index.h>
}
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>


enum QueryType {
	QUERY_NONE,
	QUERY_DECL,
	QUERY_IMPL
};

int main(int argc, char* argv[]) {
	if (argc < 4) {
		fprintf(stderr, "Usage:\n\t <file> <line> <col> <-d| -i> [<options>] \n");
		return 1;
	}

	char* file_name = argv[1];
	int line = atoi( argv[2] );
	int col = atoi( argv[3] );
	int query_type = QUERY_NONE;

	// get options
	int c = getopt(argc, argv, "dia:");
    switch (c) {
        case 'd':
            query_type = QUERY_DECL;
            break;
        case 'i':
            query_type = QUERY_IMPL;
            break;
        default:
            printf("unknow option -%c.\n", optopt);
    }
    assert( query_type != QUERY_NONE);
			    
	// debug print    
	std::cout << "file = " << file_name << "\n";
	std::cout << "line = " << line << ", col=" << col << ", type = " << query_type << "\n";

	// Set up the clang translation unit
	CXIndex cxindex = clang_createIndex(0, 0);
	CXTranslationUnit tu;
    CXErrorCode error = clang_parseTranslationUnit2(
		cxindex, 
        file_name,
		argv+5, 
        argc-5,
		nullptr, 
        0,
		CXTranslationUnit_DetailedPreprocessingRecord,
        &tu);
    if (error != CXError_Success ) {
        std::cerr << "clang_parseTranslationUnit failed: "<< error << std::endl;
        return 1;
    }

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

	CXFile cx_file = clang_getFile(tu, (const char*)file_name);
	CXSourceLocation cx_source_loc = clang_getLocation(tu, cx_file, line, col);
	CXCursor cx_cursor = clang_getCursor(tu, cx_source_loc);
	CXCursor t_cursor = clang_getNullCursor();
	if (query_type == QUERY_IMPL)
    {
		t_cursor = clang_getCursorDefinition(cx_cursor);
    }
	else //query_type is QUERY_DECL
    {
		t_cursor = clang_getCursorReferenced(cx_cursor);
    }
	unsigned int cx_line, cx_col, cx_offset;
	clang_getExpansionLocation(
		clang_getCursorLocation(cx_cursor),
		&cx_file, &cx_line, &cx_col, &cx_offset);
	if (clang_getFileName(cx_file).data) {
		printf("current file: %s, line: %d, col: %d\n",
		       clang_getCString(clang_getFileName(cx_file)),
		       cx_line,
		       cx_col);
			
		CXString cx_usr = clang_getCursorUSR(cx_cursor);
		printf("current display = %s\n", clang_getCString(cx_usr));
	}

	CXFile t_file;
	unsigned int t_line, t_col, t_offset;
	clang_getExpansionLocation(
		clang_getCursorLocation(t_cursor),
		&t_file, &t_line, &t_col, &t_offset);

	if (clang_getFileName(t_file).data) {
		printf("target file: %s, line: %d, col: %d\n",
		       clang_getCString(clang_getFileName(t_file)),
		       t_line,
		       t_col);
			
		CXString t_usr = clang_getCursorUSR(t_cursor);
		printf("target display = %s\n", clang_getCString(t_usr));
	}
	else printf("no target file found.\n");


	// dealloc
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(cxindex);

	return 0;
}
