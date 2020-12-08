#pragma once

typedef struct FlagDef {
	FlagDef() {
		name[0] = getter[0] = '\0';
	}
	char name [128];
	char getter [128];
} FlagDef;

typedef struct FlagSetDef {
	union {
		char prefix [128];
		char set [128];
	};
	std::list<FlagDef> flags;
} FlagSetDef;

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
} VarDef;