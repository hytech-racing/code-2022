#include "Parser.h"
#include "VarDef.h"

void Parser::parseFlag() {
	FlagSetDef fsdef;
	FlagDef fdef;
	if (!input.getParam(fsdef.set))
		return;
	if (input.eat(',') && !input.getParam(fdef.name))
		return;
	if (!input.eat(')'))
		return;
	puts("FLAGDEF:");
	printf("SET:\t\t%s\n", fsdef.set);
	printf("NAME:\t\t%s\n", fdef.name);
	printf("GETTER:\t\t%s\n\n", fdef.getter);
}

void Parser::parseFlagNameline() {
	eatToken(input, "inline");
	input.eatspace();
	if (!eatToken(input, "bool"))
		return;
	FlagDef fdef;
	input.getToken(fdef.getter);
	if (strempty(fdef.name) && strstart(fdef.getter, "get_"))
		strcpy(fdef.name, fdef.getter + ct_strlen("get_"));

	puts("FLAGDEF:");
	printf("NAME:\t\t%s\n", fdef.name);
	printf("GETTER:\t\t%s\n\n", fdef.getter);

}