#include "Parser.h"
#include "VarDef.h"

void Parser::parseFlag() {
	FlagSetDef fsdef;
	FlagDef fdef;

	input.getParam(fsdef.set, FLAG);
	if (input.eat(','))
		input.getParam(fdef.name, NAME);
	closeParen();

	for (FlagSetDef& other : floaters)
		if (streq(fsdef.set, other.set)) {
			other.flags.push_back(fdef);
			currentFlag = &other.flags.back();
			return;
		}
	fsdef.flags.push_back(fdef);
	floaters.push_back(fsdef);
	currentFlag = &floaters.back().flags.back();
}

void Parser::parseFlagNameline() {
	eatToken(input, "inline");
	input.eatspace();
	if (!eatToken(input, "bool"))
		throw InvalidDatatypeException("bool");

	input.getToken(currentFlag->getter);
	if (strempty(currentFlag->name) && strstart(currentFlag->getter, "get_"))
		strcpy(currentFlag->name, currentFlag->getter + ct_strlen("get_"));
}