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
		if (pipelined && strempty(outfilepath)) {
			perror("Cannot show help menu if output is being redirected\n");
			return 0;
		}
		showMenu(argv[0]);
		return 0;
	}

	if (strempty(infilepath) && !pipelined) {
		perror("Must specify either input path or --pipelined flag\n");
		return 0;
	}

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
    uint32_t id, len;
	char timeString [1024]; // it's overly long so I can skip the first line
	uint8_t data [8];

	fgets(timeString, 1024, infile); // pop off header

	fputs("time,id,message,label,value,unit\n", outfile);

	uint64_t timeRaw; uint32_t ms;

	while (!feof(infile)) {
		if (fscanf(infile, "%lu.%u,%x,%u,%lx", &timeRaw, &ms, &id, &len, (uint64_t*) data) == EOF)
			continue;

		strftime(timeString, 32, "%Y-%m-%dT%H:%M:%S", gmtime((time_t*) &timeRaw));
		sprintf(timeString + ct_strlen("YYYY-MM-DDTHH:MM:SS"), ".%03dZ", ms);

		if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ && infile != stdin) // Raw SD data is big endian
			for (int i = 0; i < 4; ++i)
				std::swap(data[i], data[7 - i]);

		parseMessage(id, timeString, data);
	}

	fcloseall();
}
