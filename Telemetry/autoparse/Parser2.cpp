#include "Parser.h"
#include "VarDef.h"

void Parser::parseVarNameline() {
	input.eat('u');
	if (eatToken(input, "int")) {
		int size = input.getInt();
		if (isPow2(size) && isValidDataLength(size)) {
			if (eatToken(input, "_t") && input.getToken(vars.back().name));
				// printf("VARNAME: %s\n\n", stash);
		}
	}
}

void Parser::parseVar() {
	VarDef vdef;
	while (input.find('@')) {
		input.get();
		if (eatToken(input, NAME)) {
			if (!input.eat('(') || !input.getParam(vdef.name) || !input.eat(')'))
				return;
		}
		else if (eatToken(input, GETTER)) {
			if (!input.eat('(') || !input.getParam(vdef.name) || !input.eat(')'))
				return;
		}
		else if (eatToken(input, HEX)) {
			if (input.eatspace() || (input.eat('(') && input.eat(')')))
				vdef.hex = true;
			else
				return;
		}
		else if (eatToken(input, UNIT)) {
			if (!input.eat('(') || !input.getParam(vdef.unit) || !input.eat(')'))
				return;
		}
		else if (eatToken(input, SCALE)) {
			if (!input.eat('('))
				return;
			vdef.scale = input.getInt();
			if (vdef.scale == 0)
				return;
			if (input.eat(',')) {
				vdef.precision = input.getInt();
				if (vdef.precision == 0)
					return;
			}
			if (!input.eat(')'))
				return;
		}
		else if (eatToken(input, FLAGPREFIX)) {
			if (vdef.flags == nullptr)
				vdef.flags = new FlagSetDef;
			if (input.eatspace())
				strcpy(vdef.flags->prefix, "-");
			else if (!input.eat('('))
				return;
			else if (input.eat(')'))
				strcpy(vdef.flags->prefix, "-");
			else if (!input.getParam(vdef.flags->prefix) || !input.eat(')'))
				return;
		}
		else if (eatToken(input, FLAGLIST)) {
			if (!input.eat('('))
				return;
			if (vdef.flags == nullptr)
				vdef.flags = new FlagSetDef;
			do {
				FlagDef fdef;
				if (!input.getToken(fdef.name))
					return;
				if (input.eat('(')) {
					if (!input.getParam(fdef.getter) || !input.eat(')'))
						return;
				}
				else {
					strcpy(fdef.getter, "get_");
					strcpy(fdef.getter + ct_strlen("get_"), fdef.name);
				}
				vdef.flags->flags.push_back(fdef);
			} while (input.eat(','));
			if (!input.eat(')')) {
				delete vdef.flags;
				vdef.flags = nullptr;
			}
		}
		else if (eatToken(input, FLAGSET)) {
			if (input.eatspace() || (input.eat('(') && input.eat(')')))
				if (vdef.flags == nullptr)
					vdef.flags = new FlagSetDef;
		}
	}

	vars.push_back(vdef);
}