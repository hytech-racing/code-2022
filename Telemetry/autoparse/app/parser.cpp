#include "AutoParse.h"
#include "StringUtils.h"
#include "Token.h"

#include <algorithm>
#include <ctime>
#include <getopt.h>
#include <stdio.h>
#include <iostream>

void showMenu(char* exe) {
	puts("\nHyTech SD Parsing System");
	puts("------------------------");
	printf("Usage: %s [options]\n", exe);
	puts("Options:");
	puts("  -h, --help          Shows this message");
	puts("  -p, --pipelined     Use stdin for input file, stdout for output file");
	puts("  -v, --verbose       Verbose (print all parsed datapoints)");
	puts("  -i [inpath]         Input filepath");
	puts("  -o [outpath]        Output filepath (defaults to inpath + _parsed)\n");
}

FILE* outfile;
int verbose = 0;

void run(FILE* infile);

int main(int argc, char** argv) {
	int help_flag = 0, pipelined = 0;
	char infilepath[128] = "";
	char outfilepath[128] = "";

	const struct option long_options[] = {
		{"help",		no_argument,	&help_flag, 1},
		{"pipelined",	no_argument,	&pipelined, 1},
		{"verbose",		no_argument,	&verbose, 1},
		{0, 0, 0, 0}
	};

	char c;
	while ((c = getopt_long(argc, argv, "i:o:hpv", long_options, nullptr)) != -1) {
		switch (c) {
			case 0: break;
			case 'i': attemptCopy(infilepath, optarg, "Input File (-i)"); break;
			case 'o': attemptCopy(outfilepath, optarg, "Output File (-o)"); break;
			case 'h': help_flag = true; break;
			case 'p': pipelined = true; break;
			case 'v': verbose = true; break;
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
			strcpy(outfilepath, infilepath);
			strcpy(outfilepath + strlen(infilepath) - ct_strlen(".csv"), "_parsed.csv");
		}
		if (!(outfile = fopen(outfilepath, "w"))) {
			perror("Unable to open output file\n");
			return 0;
		}
	}

	FILE* infile = strempty(infilepath) ? stdin : fopen(infilepath, "r");
	if (!infile) {
		perror("Unable to open input file\n");
		return 0;
	}
	run(infile);
	return 0;
}

void run (FILE* infile) {
    uint32_t id, len;
	char timeString [1024]; // it's overly long so I can skip the first line
	uint8_t data [8];

	fgets(timeString, 1024, infile); // pop off header

	fputs("time,id,message,label,value,unit\n", outfile);

	if (outfile == stdout)
		fflush(outfile);

	uint64_t timeRaw; uint32_t ms;

	while (!feof(infile)) {
		if (fscanf(infile, "%lu,%x,%u,%lx", &timeRaw, &id, &len, (uint64_t*) data) == EOF)
			continue;
		// split ms time to seconds for processing
		ms = timeRaw % 1000;
		timeRaw /= 1000;

		strftime(timeString, 32, "%Y-%m-%dT%H:%M:%S", gmtime((time_t*) &timeRaw));
		sprintf(timeString + ct_strlen("YYYY-MM-DDTHH:MM:SS"), ".%03dZ", ms);

		#if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__) // handle endianness change
			for (uint8_t *l = data, *r = data + len - 1; l < r; std::swap(*l++, *r--));
		#endif

		parseMessage(id, timeString, data);

		if (outfile == stdout)
			fflush(outfile);
	}

	exit(0);
}
