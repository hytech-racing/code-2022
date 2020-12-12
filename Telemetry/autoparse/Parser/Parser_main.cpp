#include "Parser.h"

#include "Canonicalize.h"

Parser::Parser(const char* const filepath, int bufferlength) : input(filepath, bufferlength) {}

Parser::~Parser() {
	for (ClassDef* cdef : classdefs)
		delete cdef;
	for (VarDef* vdef : vars)
		delete vdef;
}

void Parser::run() {
	char* commentStart;

	while (commentStart = input.find('/')) {
		if (!validComment(commentStart))
			continue;
		
		ParseType type = getType();
		switch (type) {
			case ParseType::Variable: parseVar(); break;
			case ParseType::Class: parseClass(); break;
			case ParseType::Flag: parseFlag(); break;
		}

		input.setStopMode(StopMode::FILE);
		input.getline();
	}

	Canonicalize::classDefs(classdefs, defaultClassProps);
	Canonicalize::mapFlagDefs(floaters, vars);
	Canonicalize::varDefs(vars);
}

bool Parser::validComment(char* const commentStart) {
	samelineComment = commentStart != input.lineStart();

	char c = input.get();
	if (c != '/' && c != '*')
		return false;
	
	if (samelineComment && c == '*' && !strstr(commentStart, M_COMMENT_CLOSE)) {
		input.skip(StopMode::COMMENT);
		return false;
	}

	input.setStopMode(samelineComment || c == '/' ? StopMode::LINE : StopMode::COMMENT);
	return true;
}

ParseType Parser::getType () {
	while (input.find('@')) {
		input.get();
		if (eatToken(input, PARSE)) {
			if (input.eatspace())
				return ParseType::Variable;
			if (eatToken(input, CLASS) && input.eatspace())
				return ParseType::Class;
			if (eatToken(input, FLAG) && input.eat('('))
				return ParseType::Flag;
		}
	}
	return ParseType::None;
}

void Parser::loadNameline() {
	if (samelineComment) {
		input.setStopMode(StopMode::LINE);
		input.restartLine();
	}
	else {
		input.setStopMode(StopMode::FILE);
		input.getline();
	}
}

Writer Parser::getWriter() {
	return Writer(defaultClassProps.name, classdefs, vars);
}