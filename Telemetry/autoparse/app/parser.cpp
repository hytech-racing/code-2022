#include "AutoParse.h"
#include "StringUtils.h"
#include "Token.h"

#include <algorithm>
#include <ctime>
#include <getopt.h>

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

FILE* outfile;
void run(FILE* infile);

int main(int argc, char** argv) {
	int help_flag = 0, pipelined = 0;
	char infilepath[128] = "";
	char outfilepath[128] = "";

	const struct option long_options[] = {
		{"help",		no_argument,	&help_flag, 1},
		{"pipelined",	no_argument,	&pipelined, 1},
		{0, 0, 0, 0}
	};

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
		if (pipelined && strempty(outfilepath))
			throw "Cannot show help menu if output is being redirected";
		showMenu(argv[0]);
		return 0;
	}

	if (strempty(infilepath) && !pipelined)
		throw "Must specify either input path or --pipelined flag";

	if (pipelined && strempty(outfilepath))
		outfile = stdout;
	else {
		if (strempty(outfilepath)) {
			sscanf(infilepath, "%s.", outfilepath);
			strcat(outfilepath, "_parsed.csv");
		}
		outfile = fopen(outfilepath, "w");
	}

	run(strempty(infilepath) ? stdin : fopen(infilepath, "r"));
	return 0;
}

void run (FILE* infile) {
    uint32_t id;
	char timeString [1024]; // it's overly long so I can skip the first line
	uint8_t data [8];

	if (infile != stdin) // pop off header if csv
		fgets(timeString, 1024, infile);

	fputs("time,id,message,label,value,unit", outfile);

	uint64_t timeRaw;

	while (fscanf(infile, "%lu,%x,%lx", &timeRaw, &id, (uint64_t*) data) != EOF) {
		int ms = timeRaw % 1000;
		timeRaw /= 1000;
		size_t len = strftime(timeString, 32, "%Y-%m-%dT%H:%M:%S", gmtime((time_t*) &timeRaw));
		sprintf(timeString + len, ".%03dZ", ms);

		if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ && infile != stdin) // Raw SD data is big endian
			for (int i = 0; i < 4; ++i)
				std::swap(data[i], data[7 - i]);

		parseMessage(id, timeString, data);
	}

	fcloseall();
}
