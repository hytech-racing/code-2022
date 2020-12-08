#include "Parser.h"

Parser::Parser(const char* const filepath, int bufferlength) :
	input(filepath, bufferlength),
	stash(new char[bufferlength]) {}

Parser::~Parser() {
	delete [] stash;
}

void Parser::run() {
	bool multiline;
	int len;

	char* commentStart;
	while (commentStart = input.find('/')) {
		bool sameline = commentStart != input.lineStart();

		char c = input.get();
		if (c != '/' && c != '*')
			continue;
		
		multiline = c == '*';

		if (sameline && multiline && !strstr(commentStart, M_COMMENT_CLOSE)) {
			input.skip();
			continue;
		}
		if (sameline)
			input.stash(stash);

		input.setStopMode(multiline ? StopMode::COMMENT : StopMode::LINE);

		ParseType type = getType();

		switch (type) {
			case ParseType::Variable: parseVar(); break;
			case ParseType::Class: parseClass(); break;
			case ParseType::Flag: parseFlag(); break;
			default: puts("NONE");
		}

		input.setStopMode(StopMode::FILE);

		if (sameline)
			input.load(stash);
		else
			input.getline();

		switch (type) {
			case ParseType::Variable: parseVarNameline(); break;
			case ParseType::Class: parseClassNameline(); break;
			case ParseType::Flag: parseFlagNameline(); break;
		}

		input.setStopMode(StopMode::FILE);
		input.getline();
	}
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