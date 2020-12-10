#pragma once

#include "ClassDef.h"
#include "VarDef.h"
#include "InputBuffer.h"
#include "Token.h"
#include "Exception.h"

class Parser {
public:
	Parser(const char* const filepath, int bufferlength = BUFFER_LENGTH);
	~Parser();
	void run();

	friend int main();
private:
	InputBuffer input;
	bool samelineComment;

	char classname [128];
	std::list<ClassDef*> classdefs;
	std::list<VarDef*> vars;
	std::list<FlagSetDef*> floaters;
	FlagDef* currentFlag;
	
	bool validComment(char* const commentStart);
	ParseType getType();
	void loadNameline();

	void parseClass();
	void parseClassDefParams(char* target, char* optional, const char* const TOK1, const char* const TOK2);
	void parseClassNameline();
	void addClassDef(ClassDef* cdef);

	void parseVar();
	void parseVarFlagPrefix(VarDef* vdef);
	void parseVarFlagList(VarDef* vdef);
	void parseVarNameline();

	void parseFlag();
	void parseFlagNameline();

	void getOneParam(char* target, const char* const TOKEN);
	bool checkNoParam();

	inline void openParen() { if (!input.eat('(')) throw SyntaxException('('); }
	inline void closeParen() { if (!input.eat(')')) throw SyntaxException(')'); }
	inline void getIntParam(int& target, const char* const TOKEN) {
		if (target)
			throw ReassignmentException(TOKEN, target);
		if (!(target = input.getInt())) 
			throw InvalidParameterException(TOKEN);
	}
};