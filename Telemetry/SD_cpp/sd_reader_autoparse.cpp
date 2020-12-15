#include "AutoParse.h"
#include "Token.h"
#include <algorithm>
#include <ctime>

#define DATE_FORMAT "YYYY-MM-DD HH:mm:ss"
#define DATE_FORMAT_MS DATE_FORMAT ".qqq"

#define FLIP_ENDIANNESS true

FILE* outfile;

int main (int argc, char** argv) {
	char* outputFilepath;

	if (argc == 3) {
		outputFilepath = argv[2];
	}
	else if (argc == 2) {
		sscanf(argv[1], "%s.", outputFilepath);
		strcat(outputFilepath, "_parsed.csv");
		printf("Auto-generating output file %s\n", outputFilepath);
	}
	else {
		printf("Usage: %s <input file> <output file (optional)>\n", argv[0]);
		return 0;
	}

	FILE* infile = fopen(argv[1], "r");
	outfile = fopen(outputFilepath, "w");

	fputs("timestamp,id,message,label,value,unit", outfile);

    uint32_t id;

	uint64_t timeRaw;
	struct tm* timeStruct;
	char timeString [] { DATE_FORMAT_MS };
	
	uint8_t data [8];

	while (fscanf(infile, "%lu,%x,%lx", &timeRaw, &id, (uint64_t*) data) != EOF) {
		int ms = timeRaw % 1000;
		timeRaw /= 1000;
		timeStruct = gmtime((time_t*) &timeRaw);
		strftime(timeString, ct_strlen(DATE_FORMAT_MS), "%Y-%m-%d %H:%M:%S", timeStruct);
		sprintf(timeString + ct_strlen(DATE_FORMAT), ".%d", (int) (timeRaw % 1000));

		#if FLIP_ENDIANNESS
			for (int i = 0; i < 4; ++i)
				std::swap(data[i], data[7 - i]);
		#endif

		parseMessage(id, timeString, data);
	}
}