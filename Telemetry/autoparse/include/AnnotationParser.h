#pragma once

#include "ClassDef.h"
#include "VarDef.h"
#include "InputBuffer.h"
#include "Token.h"
#include "Exception.h"
#include "Writer.h"

class AnnotationParser {
public:
	AnnotationParser(const char* const filepath, int bufferlength = BUFFER_LENGTH);
	~AnnotationParser();
	void run();
	Writer getWriter();

	friend void run(char* filename);
private:
	InputBuffer input;
	bool samelineComment;

	ClassDef defaultClassProps;
	std::list<ClassDef*> classdefs;
	std::list<VarDef*> vars;
	std::list<FlagSetDef*> floaters;
	
	bool validComment(char* const commentStart);
	ParseType getType();
	void loadNameline();

	void parseClass();
	void parseClassDef(ClassDef* &cdef);
	void parseClassDefParams(char* target, char* optional, const char* const TOK1, const char* const TOK2);
	void parseClassNameline();
	void addClassDef(ClassDef* cdef);

	void parseVar();
	void parseVarDef(VarDef* vdef);
	void parseVarFlagPrefix(VarDef* vdef);
	void parseVarFlagList(VarDef* vdef);
	void parseVarNameline(char* target);

	void parseFlag();
	void parseFlagNameline(FlagDef* fdef);

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