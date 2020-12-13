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

	// printf("\nGenerated Parser Config for %s\n", classname);
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

		fprintf(source, "\t%s data(buf);\n", classname);
		if (!strempty(cdef->custom)) {
			fprintf(source, "\t%s(%s, timestamp, data);\n", cdef->custom, cdef->id);
			fprintf(userDefined, "void %s (int id, const char* const timestamp, %s& data);\n", cdef->custom, classname);
		}
		char* prefix_iter = prefix + sprintf(prefix, "printf(stringify(%s) \",%s,", cdef->id, cdef->name);
		addPrefix(prefix_iter, cdef->prefix);

		for (VarDef* vdef : vars) {
			if (vdef->hex || vdef->flags)
				writeFlagParser(vdef, prefix_iter);
			else
				writeNumericalParser(vdef);
		}
		fputs("}\n\n", source);
	}
}

void Writer::writeNumericalParser(VarDef* vdef) {
	startLine();
	fprintf(source, "%s,", vdef->name);

	if (vdef->scale) {
		if (vdef->precision)
			fprintf(source, "%%.%df", vdef->precision);
		else
		    fprintf(source, "%%f");
	}
	else
	    fprintf(source, "%%d");

	fprintf(source, ",%s\\n\", ", vdef->unit);
	fprintf(source, "data.%s", vdef->getter);

	if (vdef->scale)
		fprintf(source, " / (double) %d", vdef->scale);
	fputs(");\n", source);
}

void Writer::writeFlagParser(VarDef* vdef, char* prefix_iter) {
	if (vdef->hex) {
		startLine();
		fprintf(source, "%s,0x%%X\\n\", data.%s);\n", vdef->name, vdef->getter);
	}
	if (!vdef->flags)
		return;
	
	if (!strempty(vdef->flags->prefix))
		sprintf(prefix_iter, "%s_", vdef->flags->prefix);
	for (FlagDef* fdef : vdef->flags->flags) {
		startLine();
		fprintf(source, "%s,0x%%X\\n\", data.%s);\n", fdef->name, fdef->getter);
	}
	*prefix_iter = '\0';
}

void Writer::addPrefix(char*& prefix_iter, char* classprefix) {
	if (strempty(classprefix))
		return;
	
    bool hasParam = false;
    bool bracketed = false;

    for (char* c = classprefix; *c; ++c) {
        if (*c == '{') {
            hasParam = bracketed = true;
            prefix_iter += sprintf(prefix_iter, "%%d");
        }
        else if (*c == '}')
            bracketed = false;
        else if (!bracketed)
            *prefix_iter++ = *c;
    }
    prefix_iter += sprintf(prefix_iter, "_");
    
    if (!hasParam)
        return;
    
    fprintf(source, "\tchar prefix [128];\n");
    fprintf(source, "\tsprintf(prefix, %s\"", prefix + strlen("printf("));
    
    for (char* c = classprefix; *c; ++c) {
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
    
    prefix_iter = prefix + sprintf(prefix, "printf(prefix); printf(\"");
}