#include "Writer.h"

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
		printf("void parse_%s (const char* const timestamp, uint8_t* buf) {\n", cdef->id);
		printf("\t%s data(buf);\n", classname);
		if (!strempty(cdef->custom))
			printf("\t%s(%s, timestamp, data);\n", cdef->custom, cdef->id);

		char* prefix_iter = prefix + sprintf(prefix, "printf(stringify(%s) \",%s,", cdef->id, cdef->name);
		addPrefix(prefix_iter, cdef->prefix);

		for (VarDef* vdef : vars) {
			if (vdef->hex || vdef->flags)
				writeFlagParser(vdef, prefix_iter);
			else
				writeNumericalParser(vdef);
		}
		puts("}\n");
	}
}

void Writer::writeNumericalParser(VarDef* vdef) {
	startLine();
	printf("%s,", vdef->name);

	if (vdef->scale) {
		if (vdef->precision)
			printf("%%.%df", vdef->precision);
		else
		    printf("%%f");
	}
	else
	    printf("%%d");

	printf(",%s\\n\", ", vdef->unit);
	printf("data.%s", vdef->getter);

	if (vdef->scale)
		printf(" / (double) %d", vdef->scale);
	puts(");");
}

void Writer::writeFlagParser(VarDef* vdef, char* prefix_iter) {
	if (vdef->hex) {
		startLine();
		printf("%s,0x%%X\\n\", data.%s);\n", vdef->name, vdef->getter);
	}
	if (!vdef->flags)
		return;
	
	if (!strempty(vdef->flags->prefix))
		sprintf(prefix_iter, "%s_", vdef->flags->prefix);
	for (FlagDef* fdef : vdef->flags->flags) {
		startLine();
		printf("%s,0x%%X\\n\", data.%s);\n", fdef->name, fdef->getter);
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
    
    printf("\tchar prefix [128];\n");
    printf("\tsprintf(prefix, %s\"", prefix + strlen("printf("));
    
    for (char* c = classprefix; *c; ++c) {
        if (*c == '{') {
            bracketed = true;
            printf(", data.");
        }
        else if (*c == '}')
            bracketed = false;
        else if (bracketed)
            putchar(*c);
    }
    
    puts(");");
    
    prefix_iter = prefix + sprintf(prefix, "printf(prefix); printf(\"");
}