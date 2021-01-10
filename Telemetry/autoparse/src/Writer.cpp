#include "Writer.h"

FILE* source;
FILE* include;
FILE* parseMessage;
FILE* userDefined;

Writer::Writer(char* classname, std::list<ClassDef*> classdefs, std::list<VarDef*> vars) : 
	classdefs(classdefs), 
	vars(vars) {
	strcpy(this->classname, classname);
}

void Writer::run() {

	// printf("\nGenerated AnnotationParser Config for %s\n", classname);
	// puts("\nClass Definition(s)");
	// puts("-------------------");
	// for (ClassDef* cdef : classdefs)
	// 	cdef->print();

	// puts("\nVariable Definition(s)");
	// puts("------------------------");
	// for (VarDef* vdef : vars)
	// 	vdef->print();

	for (ClassDef* cdef : classdefs) {
		fprintf(include, "void parse_%s (const char* const timestamp, uint8_t* buf);\n", cdef->id);
		fprintf(source, "void parse_%s (const char* const timestamp, uint8_t* buf) {\n", cdef->id);
		fprintf(parseMessage, "\t\tcase %s: parse_%s(timestamp, buf); break;\n", cdef->id, cdef->id);

		fprintf(source, "\tstatic %s prev(getNeg(buf));\n", classname);
		fprintf(source, "\t%s data(buf);\n", classname);
		addPrefix(cdef);
		if (!strempty(cdef->custom)) {
			fprintf(source, "\t%s(timestamp, prefix, data);\n", cdef->custom);
			fprintf(userDefined, "void %s (const char* const timestamp, const char* const prefix, %s& data);\n", cdef->custom, classname);
		}

		for (VarDef* vdef : vars) {
			if (vdef->hex || vdef->flags)
				writeFlagParser(vdef);
			else
				writeNumericalParser(vdef);
		}
		fputs("\tprev.load(buf);", source);
		fputs("}\n\n", source);
	}
}

void Writer::writeNumericalParser(VarDef* vdef) {
	startLine(vdef->name, vdef->getter);

	if (vdef->scale)
		fprintf(source, "%%.%df", vdef->precision);
	else
	    fputs("%d", source);

	if (!strempty(vdef->unit))
		fprintf(source, ",%s", vdef->unit);

	if (vdef->scale)
		sprintf(vdef->getter + strlen(vdef->getter), " / (double) %d", vdef->scale);
	writeParams(vdef->getter);
}

void Writer::writeFlagParser(VarDef* vdef) {
	if (vdef->hex) {
		startLine(vdef->name, vdef->getter);
		fputs("0x%X", source);
		writeParams(vdef->getter);
	}
	if (!vdef->flags)
		return;
	
	bool hasPrefix = !strempty(vdef->flags->prefix);

	char fullname [256], *start;
	if (hasPrefix)
		start = fullname + sprintf(fullname, "%s_", vdef->flags->prefix);

	for (FlagDef* fdef : vdef->flags->flags) {
		startLine(hasPrefix ? strcpy(start, fdef->name) : fdef->name, fdef->getter);
		fputs("%d", source);
		writeParams(fdef->getter);
	}
}

void Writer::addPrefix(ClassDef* cdef) {
    bool hasParam = false;
    bool bracketed = false;

	char prefix [1024];
	char* prefix_iter = prefix + sprintf(prefix, "stringify(%s) \",%s,", cdef->id, cdef->name);

    for (char* c = cdef->prefix; *c; ++c) {
        if (*c == '{') {
            hasParam = bracketed = true;
            prefix_iter += sprintf(prefix_iter, "%%d");
        }
        else if (*c == '}')
            bracketed = false;
        else if (!bracketed)
            *prefix_iter++ = *c;
    }
    
	if (!strempty(cdef->prefix))
		prefix_iter += sprintf(prefix_iter, "_");
    
    if (!hasParam) {
		fprintf(source, "\tstatic constexpr const char* const prefix = %s\";\n", prefix);
        return;
	}
    
    fprintf(source, "\tchar prefix [128];\n");
    fprintf(source, "\tsprintf(prefix, %s\"", prefix);
    
    for (char* c = cdef->prefix; *c; ++c) {
        if (*c == '{') {
            bracketed = true;
            fprintf(source, ", data.");
        }
        else if (*c == '}')
            bracketed = false;
        else if (bracketed)
            fputc(*c, source);
    }
    
    fputs(");\n", source);
    
    prefix_iter = prefix + sprintf(prefix, "fprintf(outfile, \"%%s\", prefix); fprintf(outfile, \"");
}