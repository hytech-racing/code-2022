#include "Parser.h"

Parser::Parser(const char* const filepath, int bufferlength) :
	input(filepath, bufferlength) {}

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

	printf("Generated Parser Config for %s\n", classname);
	puts("\nClass Definition(s)");
	puts("-------------------");
	for (ClassDef* cdef : classdefs)
		cdef->print();

	for (FlagSetDef* fsdef : floaters) {
		bool ok = false;
		for (VarDef* vdef : vars) {
			if (vdef->flags && streq(vdef->name, fsdef->set)) {
				while (fsdef->flags.size()) {
					vdef->flags->flags.push_back(fsdef->flags.front());
					fsdef->flags.pop_front();
				}
				ok = true;
				break;
			}
		}
		if (!ok)
			throw UnmappedFlagException(fsdef->set);
		delete fsdef;
	}

	puts("\nVariable Definition(s)");
	puts("------------------------");
	for (VarDef* vdef : vars)
		vdef->print();
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

	input.setStopMode(samelineComment ? StopMode::LINE : StopMode::COMMENT);
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