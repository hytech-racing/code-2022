#include "AnnotationParser.h"
#include "VarDef.h"

#include <cmath>

void AnnotationParser::parseVar() {
	VarDef* vdef = new VarDef();
	try {
		parseVarDef(vdef);
		if (strempty(vdef->name)) {
			loadNameline();
			parseVarNameline(vdef->name);
		}
		else if (strempty(vdef->getter)) {
			loadNameline();
			strcpy(vdef->getter, GET_PREFIX);
			parseVarNameline(vdef->getter + ct_strlen(GET_PREFIX));
		}
		vars.push_back(vdef);
	} catch (std::exception const& e) {
		delete vdef;
		throw;
	}
}

void AnnotationParser::parseVarNameline(char* const target) {
	if (eatToken(input, "bool") && input.getToken(target))
		return;
	input.eat('u');
	if (eatToken(input, "int")) {
		int size = input.getInt();
		if (isPow2(size) && isValidDataLength(size) && eatToken(input, "_t") && input.getToken(target))
			return;
	}
	throw InvalidDatatypeException("int_t or uint_t (8, 16, 32, or 64)");
}

void AnnotationParser::parseVarDef(VarDef* vdef) {
	while (input.find('@')) {
		input.get();
		if (eatToken(input, NAME))			getOneParam(vdef->name, NAME);
		else if (eatToken(input, GETTER))	getOneParam(vdef->getter, GETTER);
		else if (eatToken(input, HEX))		vdef->hex = checkNoParam();
		else if (eatToken(input, UNIT))		getOneParam(vdef->unit, UNIT);
		else if (eatToken(input, SCALE)) {
			openParen();
			getIntParam(vdef->scale, SCALE);
			if (input.eat(','))
				getIntParam(vdef->precision, PRECISION);
			else
				vdef->precision = log10(vdef->scale);
			closeParen();
		}
		else if (eatToken(input, FLAGPREFIX))	parseVarFlagPrefix(vdef);
		else if (eatToken(input, FLAGLIST))		parseVarFlagList(vdef);
		else if (eatToken(input, FLAGSET)) {
			checkNoParam();
			if (vdef->flags == nullptr)
				vdef->flags = new FlagSetDef();
		}
	}
}

void AnnotationParser::parseVarFlagPrefix(VarDef* vdef) {
	if (vdef->flags == nullptr)
		vdef->flags = new FlagSetDef();

	bool space = input.eatspace();
	if (input.eat('(')) {
		if (input.eat(')'))
			attemptCopy(vdef->flags->prefix, "-", FLAGPREFIX);
		else {
			input.getParam(vdef->flags->prefix, FLAGPREFIX);
			closeParen();
		}
	}
	else if (space)
		strcpy(vdef->flags->prefix, "-");
	else
		throw InvalidParameterException(FLAGPREFIX);
}

void AnnotationParser::parseVarFlagList(VarDef* vdef) {
	openParen();

	if (vdef->flags == nullptr)
		vdef->flags = new FlagSetDef();

	do {
		FlagDef* fdef = new FlagDef();
		vdef->flags->flags.push_back(fdef);
		if (!input.getToken(fdef->name))
			throw InvalidParameterException(FLAGLIST);
		if (input.eat('(')) {
			input.getParam(fdef->getter, GETTER);
			closeParen();
		}
	} while (input.eat(','));

	closeParen();
}