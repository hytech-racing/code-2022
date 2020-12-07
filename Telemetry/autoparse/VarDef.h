#pragma once

typedef struct FlagDef {
	~FlagDef() {
		for (std::pair<char*,char*> p : flags) {
			delete [] p.first;
			delete [] p.second;
		}
	}
	char prefix [128];
	std::list<std::pair<char*, char*>> flags;
} FlagDef;

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
	FlagDef* flags;
	bool hex;
} VarDef;