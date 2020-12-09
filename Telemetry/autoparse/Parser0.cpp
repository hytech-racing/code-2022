#include "Parser.h"

Parser::Parser(const char* const filepath, int bufferlength) :
	input(filepath, bufferlength),
	stash(new char[bufferlength]) {}

Parser::~Parser() {
	delete [] stash;
	for (VarDef& vdef : vars)
		if (vdef.flags)
			delete vdef.flags;
}

void Parser::run() {
	char* commentStart;
	while (commentStart = input.find('/')) {
		bool oneline = commentStart != input.lineStart();

		char c = input.get();
		if (c != '/' && c != '*')
			continue;
		
		if (oneline) {
			if (c == '*' && !strstr(commentStart, M_COMMENT_CLOSE)) {
				input.skip();
				continue;
			}
			input.stash(stash);
			input.setStopMode(StopMode::LINE);
		}
		else
			input.setStopMode(StopMode::COMMENT);
		
		ParseType type = getType();
		switch (type) {
			case ParseType::Variable: parseVar(); break;
			case ParseType::Class: parseClass(); break;
			case ParseType::Flag: parseFlag(); break;
			case ParseType::None: continue;
		}

		input.setStopMode(StopMode::FILE);
		if (oneline) input.load(stash);
		else input.getline();

		switch (type) {
			case ParseType::Variable: parseVarNameline(); break;
			case ParseType::Class: parseClassNameline(); break;
			case ParseType::Flag: parseFlagNameline(); break;
		}
		
		input.setStopMode(StopMode::FILE);
		input.getline();
	}

	printf("Generated Parser Config for %s\n", classname);
	puts("\nClass Definition(s)");
	puts("-------------------");
	for (ClassDef& cdef : classdefs)
		cdef.print();

	for (FlagSetDef& fsdef : floaters) {
		for (VarDef& vdef : vars) {
			if (vdef.flags && streq(vdef.name, fsdef.set))
				for (FlagDef& fdef : fsdef.flags)
					vdef.flags->flags.push_back(fdef);
		}
	}

	puts("\nVariable Definition(s)");
	puts("------------------------");
	for (VarDef& vdef : vars)
		vdef.print();
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