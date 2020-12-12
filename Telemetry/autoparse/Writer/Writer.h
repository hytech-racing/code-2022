#pragma once

#include <list>

#include "ClassDef.h"
#include "VarDef.h"

class Writer {
public:
	Writer(char* classname, std::list<ClassDef*> classdefs, std::list<VarDef*> vars);
	void run();
private:
	char prefix [1024];
	char classname [128];
	std::list<ClassDef*> classdefs;
	std::list<VarDef*> vars;

	void writeNumericalParser(VarDef* vdef);
	void writeFlagParser(VarDef* vdef, char* prefix_iter);
	void addPrefix(char*& prefix_iter, char* prefix);

	inline void startLine() {
		printf("\tprintf(timestamp); putchar(','); %s", prefix);
	}
};