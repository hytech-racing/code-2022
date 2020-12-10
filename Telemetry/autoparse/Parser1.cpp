
#include "Parser.h"
#include "Utils.h"

void Parser::parseClassNameline() {
	if (!eatToken(input, CLASS) || !input.getToken(classname))
		classname[0] = '\0';
}

void Parser::parseClass() {
	ClassDef defaultProps;

	while (input.find('@')) {
		ClassDef cdef;
		input.get();
		if (eatToken(input, ID)) {
			parseClassDefParams(cdef.id, cdef.name, ID, NAME);
			if (strempty(cdef.name))
				strcpy(cdef.name, "-");
			cdef.addSelfTo(classdefs);
		}
		else if (eatToken(input, PREFIX)) {
			parseClassDefParams(cdef.prefix, cdef.id, PREFIX, ID);
			if (!strempty(cdef.id))
				cdef.addSelfTo(classdefs);
			else
				attemptCopy(defaultProps.prefix, cdef.prefix, PREFIX);
		}
		else if (eatToken(input, CUSTOM))
			getOneParam(defaultProps.custom, CUSTOM);
	}

	for (ClassDef& def : classdefs) {
		if (strempty(def.prefix))
			strcpy(def.prefix, defaultProps.prefix);
		strcpy(def.custom, defaultProps.custom);
	}
}

void Parser::parseClassDefParams(char* target, char* optional, const char* const TOK1, const char* const TOK2) {
	openParen();
	input.getParam(target, TOK1);
	if (input.eat(','))
		input.getParam(optional, TOK2);
	closeParen();
}