
#include "Parser.h"
#include "Utils.h"

void Parser::parseClassNameline() {
	if (!eatToken(input, CLASS) || !input.getToken(classname))
		classname[0] = '\0';
}

void Parser::parseClass() {
	ClassDef defaultProps;
	ClassDef* cdef;

	while (input.find('@')) {
		input.get();
		if (eatToken(input, ID)) {
			cdef = new ClassDef;
			parseClassDefParams(cdef->id, cdef->name, ID, NAME);
			if (strempty(cdef->name))
				strcpy(cdef->name, "-");
			addClassDef(cdef);
		}
		else if (eatToken(input, PREFIX)) {
			cdef = new ClassDef;
			parseClassDefParams(cdef->prefix, cdef->id, PREFIX, ID);
			if (!strempty(cdef->id))
				addClassDef(cdef);
			else {
				attemptCopy(defaultProps.prefix, cdef->prefix, PREFIX);
				delete cdef;
			}
		}
		else if (eatToken(input, CUSTOM))
			getOneParam(defaultProps.custom, CUSTOM);
	}

	for (ClassDef* def : classdefs) {
		if (strempty(def->prefix))
			strcpy(def->prefix, defaultProps.prefix);
		strcpy(def->custom, defaultProps.custom);
	}
}

void Parser::parseClassDefParams(char* target, char* optional, const char* const TOK1, const char* const TOK2) {
	openParen();
	input.getParam(target, TOK1);
	if (input.eat(','))
		input.getParam(optional, TOK2);
	closeParen();
}

void Parser::addClassDef(ClassDef* cdef) {
	for (ClassDef* other : classdefs) {
		if (!streq(cdef->id, other->id))
			continue;
		if (!strempty(cdef->name))
			attemptCopy(other->name, cdef->name, NAME);
		else if (!strempty(cdef->prefix))
			attemptCopy(other->prefix, cdef->prefix, PREFIX);
		delete cdef;
		return;
	}
	classdefs.push_back(cdef);
}