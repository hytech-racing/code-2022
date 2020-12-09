#pragma once

#include "ClassDef.h"
#include "VarDef.h"
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

	char classname [128];
	std::list<ClassDef> classdefs;
	std::list<VarDef> vars;
	std::list<FlagSetDef> floaters;
	FlagDef* currentFlag;
	
	ParseType getType();

	void parseClass();
	bool parseClassID(ClassDef& cdef);
	bool parseClassPrefix(ClassDef& cdef);
	void parseClassNameline();

	void parseVar();
	void parseVarNameline();

	void parseFlag();
	void parseFlagNameline();
};