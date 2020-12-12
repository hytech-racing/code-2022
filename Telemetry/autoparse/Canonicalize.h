#pragma once
#include "ClassDef.h"
#include "VarDef.h"

namespace Canonicalize {
	void classDefs(std::list<ClassDef*>& list, ClassDef& defaultProps);
	void applyDefaultClassProps(std::list<ClassDef*>& list);
	void varDefs(std::list<VarDef*>& vars);
	void mapFlagDefs(std::list<FlagSetDef*>& flagsets, std::list<VarDef*>& vars);
}