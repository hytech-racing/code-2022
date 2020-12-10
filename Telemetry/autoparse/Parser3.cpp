#include "Parser.h"
#include "VarDef.h"

void Parser::parseFlag() {
	FlagSetDef* fsdef = new FlagSetDef;
	FlagDef* fdef = new FlagDef;

	input.getParam(fsdef->set, FLAG);
	if (input.eat(','))
		input.getParam(fdef->name, NAME);
	closeParen();

	for (FlagSetDef* other : floaters)
		if (streq(fsdef->set, other->set)) {
			other->flags.push_back(fdef);
			currentFlag = fdef;
			delete fsdef;
			return;
		}
	fsdef->flags.push_back(fdef);
	floaters.push_back(fsdef);
	currentFlag = fdef;
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