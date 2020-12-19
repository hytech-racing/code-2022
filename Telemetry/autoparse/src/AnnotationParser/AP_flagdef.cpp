#include "AnnotationParser.h"
#include "VarDef.h"

void AnnotationParser::parseFlag() {
	FlagSetDef* fsdef = new FlagSetDef;
	FlagDef* fdef = new FlagDef;

	try {
		input.getParam(fsdef->set, FLAG);
		if (input.eat(','))
			input.getParam(fdef->name, NAME);
		closeParen();

		loadNameline();
		parseFlagNameline(fdef);
	} catch (std::exception const& e) {
		delete fsdef;
		delete fdef;
		throw;
	}

	for (FlagSetDef* other : floaters)
		if (streq(fsdef->set, other->set)) {
			other->flags.push_back(fdef);
			delete fsdef;
			return;
		}
	fsdef->flags.push_back(fdef);
	floaters.push_back(fsdef);
}

void AnnotationParser::parseFlagNameline(FlagDef* fdef) {
	eatToken(input, "inline");
	input.eatspace();
	if (!eatToken(input, "bool"))
		throw InvalidDatatypeException("bool");

	input.getToken(fdef->getter);
}