#pragma once

#include <list>
#include <stdio.h>

#include "StringUtils.h"

typedef struct ClassDef {
	ClassDef() {
		id[0] = prefix[0] = name[0] = custom[0] = '\0';
	}
	char id [128];
	char name [128];
	char prefix [128];
	char custom [128];

	void print() {
		printf("ID:\t\t%s\n", id);
		printf("NAME:\t\t%s\n", name);
		printf("PREFIX:\t\t%s\n", prefix);
		printf("CUSTOM:\t\t%s\n\n", custom);
	}
} ClassDef;

typedef struct DimDef {
	DimDef() {
		getter[0] = '\0';
		maxSize = 0;
	}
	char getter [128];
	int maxSize;
} DimDef;