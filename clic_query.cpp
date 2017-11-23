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
		printf("visit FILE: %s\n", cursorFilename);
		if (!clang_getFileName(file).data) {
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

enum QueryType {
	QUERY_DECL,
	QUERY_IMPL
};

int main(int argc, char* argv[]) {
	if (argc < 7) {
		fprintf(stderr, "Usage:\n\t <-f file> <-l line> <-c col> <-d | -i> <-p path_lists>\n");
		return 1;
	}

	char* file_name;
	int col;
	int line;
	int query_type;
	char*p = NULL;
	char* tmp = NULL;
	char* path_lists[128];

	// get options
	int c;
	while ((c = getopt(argc, argv, "f:l:c:dip:")) != -1) {
		switch (c) {
		case 'f':
			file_name = optarg;
			break;
		case 'l':
			line = atoi(optarg);
			break;
		case 'c':
			col = atoi(optarg);
			break;
		case 'd':
			query_type = QUERY_DECL;
			break;
		case 'i':
			query_type = QUERY_IMPL;
			break;
		case 'p':
			tmp = optarg;
			printf("tmp = %s\n", tmp);
			break;
		default:
			printf("unknow option -%c.\n", optopt);
		}
	}
			    
	// split path lists
	p = strtok(tmp, ":");
	int cnt = 0;
	while (p != NULL) {
		path_lists[cnt++] = p;
		p = strtok(NULL, ":");
	}
	    
	// debug print    
	std::cout << "file = " << file_name << "\n";
	std::cout << "line = " << line << ", col=" << col << "\n";
	std::cout << "type = " << query_type << "\n";
	std::cout << "total " << cnt << " path lists \n";
	for (int i=0; i<cnt; i++)
		std::cout << path_lists[i] << "\n";


	// Set up the clang translation unit
	CXIndex cxindex = clang_createIndex(0, 0);
	const char* command_line_args[2] = {
		"-I/usr/lib/gcc/i486-linux-gnu/4.7/include", 
		"-I/usr/local/include"
	};
	CXTranslationUnit tu = clang_parseTranslationUnit(
		cxindex, file_name,
		command_line_args, 2,
		0, 0,
		CXTranslationUnit_DetailedPreprocessingRecord);

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
		t_cursor = clang_getCursorDefinition(cx_cursor);
	else //query_type is QUERY_DECL
		t_cursor = clang_getCursorReferenced(cx_cursor);
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
		printf("display = %s\n", clang_getCString(t_usr));
	}
	else printf("no target file found.\n");


	// dealloc
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(cxindex);

	return 0;
}
