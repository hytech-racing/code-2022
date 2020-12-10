#pragma once
#include "FlagDef.h"

typedef struct VarDef {
	VarDef() {
		name[0] = getter[0] = unit[0] = '\0';
		scale = precision = hex = 0;
		flags = nullptr;
	}
	~VarDef() {
		if (flags)
			delete flags;
	}

	char name [128];
	char getter [128];
	char unit [8];
	int scale;
	int precision;
	FlagSetDef* flags;
	bool hex;

	void print() {
		printf("NAME:\t\t%s\n", name);
		printf("GETTER:\t\t%s\n", getter);
		printf("UNIT:\t\t%s\n", unit);
		printf("SCALE:\t\t%d\n", scale);
		printf("PRECISION:\t%d\n", precision);

		if (flags)
			flags->print();

		putchar('\n');
	}
} VarDef;