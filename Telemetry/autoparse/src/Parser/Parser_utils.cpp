#include "Parser.h"
#include "Exception.h"

void Parser::getOneParam(char* target, const char* const TOKEN) {
	openParen();
	input.getParam(target, TOKEN);
	closeParen();
}

bool Parser::checkNoParam() {
	bool space = input.eatspace();
	if (input.eat('('))
		closeParen();
	else if (!space)
		throw SyntaxException(' ');
	return true;
}
