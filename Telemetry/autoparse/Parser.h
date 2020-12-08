#pragma once

#include "ClassDef.h"
#include "InputBuffer.h"
#include "Token.h"

class Parser {
public:
	Parser(const char* const filepath, int bufferlength = BUFFER_LENGTH);
	~Parser();
	void run();
private:
	InputBuffer input;
	char* const stash;
	
	ParseType getType ();
	void parseClass();
	bool parseClassID(ClassDef& cdef);
	bool parseClassPrefix(ClassDef& cdef);
	void parseClassNameline();

	void parseVar();
	void parseVarNameline();

	void parseFlag();
	void parseFlagNameline();
};