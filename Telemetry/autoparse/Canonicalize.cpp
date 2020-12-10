#include "Canonicalize.h"
#include "StringUtils.h"

namespace Canonicalize {

	namespace {
		void makeFunc(char* str) {
			if (strempty(str))
				return;
			while (*str) ++str;
			if (*(str - 1) != ')')
				strcpy(str, "()");
		}
	}

	void classDefs(std::list<ClassDef*>& list, char* classname) {
		if (strempty(classname))
			throw AutoParseException("Symbol Parseclass not found");

		if (list.empty()) {
			ClassDef* cdef = new ClassDef;
			strcpy(cdef->id, ID_PREFIX);

			char* target = cdef->id + ct_strlen(ID_PREFIX);
			for (char* src = classname; *src; ++src)
				*target++ = toupper(*src);
			*target = '\0';

			strcpy(cdef->name, classname);
			list.push_back(cdef);
		}

		for (ClassDef* cdef : list)
			if (streq(cdef->name, "-"))
				strcpy(cdef->name, classname);
	}

	void applyDefaultClassProps(std::list<ClassDef*>& list, ClassDef& defaultProps) {
		makeFunc(defaultProps.custom);
		
		for (ClassDef* def : list) {
			if (strempty(def->prefix))
				strcpy(def->prefix, defaultProps.prefix);
			strcpy(def->custom, defaultProps.custom);
		}
	}

	void varDef(std::list<VarDef*>& vars) {
		for (VarDef* vdef : vars) {
			if (strempty(vdef->getter)) {
				strcpy(vdef->getter, GET_PREFIX);
				strcpy(vdef->getter + ct_strlen(GET_PREFIX), vdef->name);
			}
			makeFunc(vdef->getter);

			if (vdef->flags) {
				if (streq(vdef->flags->prefix, "-"))
					strcpy(vdef->flags->prefix, vdef->name);
				for (FlagDef* fdef : vdef->flags->flags) {
					if (strempty(fdef->name)) {
						if (strstart(fdef->getter, GET_PREFIX))
							strcpy(fdef->name, fdef->getter + ct_strlen(GET_PREFIX));
						else
							throw AutoParseException("Expected function name starting with \"get_\"");
					}
					else if (strempty(fdef->getter)) {
						strcpy(fdef->getter, GET_PREFIX);
						strcpy(fdef->getter + ct_strlen(GET_PREFIX), fdef->name);
					}
					makeFunc(fdef->getter);
				}
			}
		}
	}

	void mapFlagDefs(std::list<FlagSetDef*>& flagsets, std::list<VarDef*>& vars) {
		for (FlagSetDef* fsdef : flagsets) {
			for (VarDef* vdef : vars) {
				if (vdef->flags && streq(vdef->name, fsdef->set)) {
					while (fsdef->flags.size()) {
						vdef->flags->flags.push_back(fsdef->flags.front());
						fsdef->flags.pop_front();
					}
					delete fsdef;
					return;
				}
			}
			throw UnmappedFlagException(fsdef->set);
		}
	}

}