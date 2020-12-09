
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
			if (parseClassID(cdef))
				cdef.addSelfTo(classdefs);
		}
		else if (eatToken(input, PREFIX)) {
			if (parseClassPrefix(cdef)) {
				if (!strempty(cdef.id))
					cdef.addSelfTo(classdefs);
				else if (!attemptCopy(defaultProps.prefix, cdef.prefix))
					puts("Duplicate default prefix");
			}
		}
		else if (eatToken(input, CUSTOM)) {
			if (!strempty(defaultProps.custom))
				puts("Repeat @CUSTOM");
			else {
				if (!input.eat('(') || !input.getParam(defaultProps.custom))
					puts("Invalid @CUSTOM");
				if (!input.eat(')'))
					defaultProps.custom[0] = '\0';
			}
		}
	}

	for (ClassDef& def : classdefs) {
		if (strempty(def.name)) {
			puts("Prefix not associated with valid ID");
			continue;
		}
		attemptCopy(def.prefix, defaultProps.prefix);
		strcpy(def.custom, defaultProps.custom);
	}
}

bool Parser::parseClassID(ClassDef& cdef) {
	if (!input.eat('(') || !input.getParam(cdef.id)) {
		puts("Invalid @ID");
		return false;
	}
	
	if (input.eat(',')) {
		if (!input.getParam(cdef.name)) {
			puts("Invalid @ID name");
			return false;
		}
	}
	else if (!strstart(cdef.id, ID_PREFIX)) {
		puts ("Unable to determine name from ID, must be provided");
		return false;
	}
	else
		strcpy(cdef.name, cdef.id + ct_strlen(ID_PREFIX));

	return input.eat(')');
}

bool Parser::parseClassPrefix(ClassDef& cdef) {
	if (!input.eat('(') || !input.getParam(cdef.prefix)) {
		puts("Invalid @PREFIX");
		return false;
	}

	if (input.eat(',')) {
		strcpy(cdef.id, cdef.prefix);
		if (!input.getParam(cdef.prefix)) {
			puts("Invalid @PREFIX");
			return false;
		}
	}

	return input.eat(')');
}