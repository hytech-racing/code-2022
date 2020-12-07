#include "Parser.h"
#include "VarDef.h"

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
				vdef.flags = new FlagDef;
			if (!input.eat('(') || !input.getParam(vdef.flags->prefix) || !input.eat(')'))
				return;
		}
		else if (eatToken(input, FLAGLIST)) {
			if (vdef.flags == nullptr)
				vdef.flags = new FlagDef;
			if (!input.eat('('))
				return;
			do {
				char* name = new char [128];
				char* getter = new char [128];
				if (!input.getToken(name)) {
					delete [] name;
					delete [] getter;
					return;
				}
				if (input.eat('(')) {
					if (!input.getParam(getter) || !input.eat(')')) {
						delete [] name;
						delete [] getter;
						return;
					}
				}
				else {
					strcpy(getter, "get_");
					strcpy(getter + ct_strlen("get_"), name);
				}
				vdef.flags->flags.push_back(std::make_pair(name, getter));
			} while (input.eat(','));
			if (!input.eat(')')) {
				delete vdef.flags;
				vdef.flags = nullptr;
			}
		}
		else if (eatToken(input, FLAGSET)) {
			bool noparam = input.eatspace();
			noparam = noparam && !input.eat('(');
			if (noparam)
				puts("FLAGSET AUTOID");
			else
				puts(FLAGSET);
		}
	}

	puts("VARDEF");
	printf("NAME:\t\t%s\n", vdef.name);
	printf("GETTER:\t\t%s\n", vdef.getter);
	printf("UNIT:\t\t%s\n", vdef.unit);
	printf("SCALE:\t\t%d\n", vdef.scale);
	printf("PRECISION:\t%d\n\n", vdef.precision);

	if (vdef.flags) {
		printf("FLAG PREFIX\t%s\n", vdef.flags->prefix);
		for (std::pair<char*, char*> p : vdef.flags->flags)
			printf("FLAG:\t\t%s %s\n", p.first, p.second);
	}
}


void Parser::parseVarNameline() {
	input.eat('u');
	if (eatToken(input, "int")) {
		int size = input.getInt();
		if (isPow2(size) && isValidDataLength(size)) {
			if (eatToken(input, "_t") && input.getToken(stash)) 
				printf("VARNAME: %s\n\n", stash);
		}
	}
}
