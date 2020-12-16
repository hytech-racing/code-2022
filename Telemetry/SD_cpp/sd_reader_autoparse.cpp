#include "AutoParse.h"
#include "StringUtils.h"
#include "Token.h"

#include <algorithm>
#include <ctime>
#include <getopt.h>

#define DATE_FORMAT "YYYY-MM-DD HH:mm:ss"
#define DATE_FORMAT_MS DATE_FORMAT ".qqq"

#define FLIP_ENDIANNESS true

FILE* outfile;

void showMenu(char* exe) {
	puts("\nHyTech SD Parsing System");
	puts("------------------------");
	printf("Usage: %s [options]\n", exe);
	puts("Options:");
	puts("  --help          Shows this message");
	puts("  --pipelined     Use stdin for input file, stdout for output file");
	puts("  -i              Input filepath");
	puts("  -o              Output filepath (defaults to -i + _parsed)\n");
}

void run (FILE* infile, FILE* __outfile) {
	outfile = __outfile;

	fputs("timestamp,id,message,label,value,unit", outfile);

    uint32_t id;

	uint64_t timeRaw;
	struct tm* timeStruct;
	char timeString [ct_strlen(DATE_FORMAT_MS)];
	
	uint8_t data [8];

	while (fscanf(infile, "%lu,%x,%lx", &timeRaw, &id, (uint64_t*) data) != EOF) {
		int ms = timeRaw % 1000;
		timeRaw /= 1000;
		timeStruct = gmtime((time_t*) &timeRaw);
		strftime(timeString, ct_strlen(DATE_FORMAT_MS), "%Y-%m-%d %H:%M:%S", timeStruct);
		sprintf(timeString + ct_strlen(DATE_FORMAT), ".%d", ms);

		#if FLIP_ENDIANNESS
			for (int i = 0; i < 4; ++i)
				std::swap(data[i], data[7 - i]);
		#endif

		parseMessage(id, timeString, data);
	}

	fcloseall();
}

int main(int argc, char** argv) {
	int help_flag = 0, pipelined = 0;

	static struct option long_options[] = {
		{"help",		no_argument,	&help_flag, 1},
		{"pipelined",	no_argument,	&pipelined, 1},
		{0, 0, 0, 0}
	};

	char infilepath[128] = "";
	char outfilepath[128] = "";

	char c;
	while ((c = getopt_long(argc, argv, "i:o:", long_options, nullptr)) != -1) {
		switch (c) {
			case 0: break;
			case 'i': attemptCopy(infilepath, optarg, "Input File (-i)"); break;
			case 'o': attemptCopy(outfilepath, optarg, "Output File (-o)"); break;
			default: return 0;
		}
    }

	if (help_flag) {
		if (pipelined)
			throw "Cannot show help menu if --pipelined=true";
		showMenu(argv[0]);
	}
	else if (pipelined) {
		if (!strempty(infilepath) || !strempty(outfilepath))
			throw "Cannot specify any other options if --pipelined=true";
		run(stdin, stdout);
	}
	else if (!strempty(infilepath)) {
		FILE* infile = fopen(infilepath, "r");

		if (strempty(outfilepath)) {
			sscanf(infilepath, "%s.", outfilepath);
			strcat(outfilepath, "_parsed.csv");
		}
		run(infile, fopen(outfilepath, "w"));
	}
	else
		showMenu(argv[0]);

	return 0;
}