#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <list>

#include "InputBuffer.h"
#include "ClassDef.h"
#include "VarDef.h"
#include "Parser.h"
#include "StringUtils.h"
#include "Writer.h"

#include <dirent.h>
#include <sys/types.h>

#define CAN_LIBRARY "../../Libraries/HyTech_CAN/"

void run(char* filename) {
	char filepath [128]; sprintf(filepath, CAN_LIBRARY "%s", filename);
	Parser p(filepath);
	try {
		p.run();
		p.getWriter().run();
	} catch (std::exception& e) {
		e.what();
		p.input.showLine(stderr);
	}
}

int main() {
	source = fopen("build/AutoParse.cpp", "a");
	include = fopen("build/AutoParse.h", "a");
	parseMessage = fopen("build/ParseMessage.cpp", "a");
	userDefined = fopen("build/UserDefined.h", "a");

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (CAN_LIBRARY)) != NULL)
		while ((ent = readdir (dir)) != NULL) {
			if (ent->d_type == DT_REG && strstr(ent->d_name, ".h"))
				run(ent->d_name);
		}
	closedir (dir);

	fputs("\t}\n}", parseMessage);
	fcloseall();

	return 0;
}

/**
 * class level
 * - ID (ID, CUSTOM_NAME?), CUSTOM_NAME defaults to lower(ID.substr(3))
 * - PREFIX (id? prefix{withgettercalls()}), written before all names. optionally map to id (noparam sets default)
 * - FUNCTION (function_name), does not force exit
 * variable level
 * - NAME(NAME), defaults to variable name
 * - GETTER(FUNC), defaults to "get_{variable_name}()"
 * - HEX, marks hex values, no param
 * - UNIT(unit name)
 * - SCALE(int, precision?), cast double, divide by int
 * - FLAGSET
 * - FLAGLIST(flag1, flag2...), assumes standard getter names
 * - FLAGPREFIX(str?), default to varname
 * function level
 * - FLAGSET(set_id, flagname?), map to flagset variable, flagname defaults to functionname minus "get_"
 */