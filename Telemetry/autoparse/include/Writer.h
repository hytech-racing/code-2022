#pragma once

#include <list>

#include "ClassDef.h"
#include "VarDef.h"

extern FILE* source;
extern FILE* include;
extern FILE* parseMessage;
extern FILE* userDefined;

class Writer {
public:
	Writer(char* classname, std::list<ClassDef*> classdefs, std::list<VarDef*> vars, std::list<DimDef*> dimList);
	void run();
private:
	char classname [128];
	char prevAccessor [128];
	std::list<ClassDef*> classdefs;
	std::list<VarDef*> vars;
	std::list<DimDef*> dimensions;

	void writeNumericalParser(VarDef* vdef);
	void writeFlagParser(VarDef* vdef);
	void addPrefix(ClassDef* cdef);

	inline void startLine(char* varname, char* getter) {
		fprintf(source, "\tif (data.%s != prev%s.%s)\n\t", getter, prevAccessor, getter);
		fputs("\tfprintf(outfile, \"%s,%s\" \"", source);
		int len;
		fprintf(source, "%s%n\"", varname, &len);
		fprintf(source, "%*s", 50 - len, "\",");
	}

	inline void writeParams(char* getter) {
		fprintf(source, "\\n\",\ttimestamp, prefix, data.%s);\n", getter);
	}
};