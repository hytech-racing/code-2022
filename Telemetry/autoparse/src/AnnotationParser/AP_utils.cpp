#include "AnnotationParser.h"
#include "Exception.h"

void AnnotationParser::getOneParam(char* target, const char* const TOKEN) {
	openParen();
	input.getParam(target, TOKEN);
	closeParen();
}

bool AnnotationParser::checkNoParam() {
	bool space = input.eatspace();
	if (input.eat('('))
		closeParen();
	else if (!space)
		throw SyntaxException(' ');
	return true;
}
