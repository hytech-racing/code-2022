#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <list>

#include "InputBuffer.h"
#include "ClassDef.h"
#include "VarDef.h"
#include "Parser.h"
#include "Utils.h"

int main() {
	Parser p("../../Libraries/HyTech_CAN/BMS_Detailed_Voltages.h");
	p.run();
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
 * - FLAGSET(set_id, flagname), map to flagset variable, flagname defaults to functionname minus "get_"
 */