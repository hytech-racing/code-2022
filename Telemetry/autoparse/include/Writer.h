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
	Writer(char* classname, std::list<ClassDef*> classdefs, std::list<VarDef*> vars);
	void run();
private:
	char classname [128];
	std::list<ClassDef*> classdefs;
	std::list<VarDef*> vars;

	void writeNumericalParser(VarDef* vdef);
	void writeFlagParser(VarDef* vdef);
	void addPrefix(ClassDef* cdef);

	inline void startLine(char* varname, int indentation = 0) {
		fputs("\tfprintf(outfile, \"%s,%s\" \"", source);
		int len;
		fprintf(source, "%s%n\"", varname, &len);
		fprintf(source, "%*s", 50 - (indentation << 2) - len, "\",");
	}

	inline void writeParams(char* getter) {
		fprintf(source, "\\n\",\ttimestamp, prefix, data.%s);\n", getter);
	}
};