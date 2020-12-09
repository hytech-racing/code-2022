#pragma once

#include <list>
#include <stdio.h>

#include "Utils.h"

typedef struct ClassDef {
	ClassDef() {
		id[0] = prefix[0] = name[0] = custom[0] = '\0';
	}
	char id [128];
	char name [128];
	char prefix [128];
	char custom [128];

	void addSelfTo(std::list<ClassDef>& list) {
		for (ClassDef& other : list) {
			if (!streq(id, other.id))
				continue;
			if (!strempty(name)) {
				if (!attemptCopy(other.name, name))
					puts("No duplicate id allowed");
			}
			else if (!strempty(prefix)) {
				if (!attemptCopy(other.name, name))
					puts("No duplicate prefix allowed");
			}
			return;
		}
		list.push_back(*this);
	}

	void print() {
		printf("ID:\t\t%s\n", id);
		printf("NAME:\t\t%s\n", name);
		printf("PREFIX:\t\t%s\n", prefix);
		printf("CUSTOM:\t\t%s\n\n", custom);
	}
} ClassDef;
