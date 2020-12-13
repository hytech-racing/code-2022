#pragma once

#include <list>
#include <stdio.h>

typedef struct FlagDef {
	char name [128];
	char getter [128];
	
	FlagDef() { name[0] = getter[0] = '\0'; }
	void print() { printf("FLAG:\t\t%s\t\t%s\n", name, getter); }
} FlagDef;

typedef struct FlagSetDef {
	union {
		char prefix [128];
		char set [128];
	};
	std::list<FlagDef*> flags;

	FlagSetDef() { prefix[0] = '\0'; }
	~FlagSetDef() {
		for (FlagDef* fdef : flags)
			delete fdef;
	}
	void print() {
		printf("FLAG PREFIX\t%s\n", prefix);
		for (FlagDef* fdef : flags)
			fdef->print();
	}
} FlagSetDef;