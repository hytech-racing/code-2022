#include "SD.h"
#include "SPI.h"

bool SD_t::begin(uint8_t cspin) {
	SPI.begin();
	if (cspin != BUILTIN_SDCARD) {
		pinMode(cspin, OUTPUT);
		digitalWrite(cspin, HIGH);
	}
	valid = true;
}

bool SD_t::exists(char* filename) {
	std::ifstream fs;
	fs.open(filename);
	bool exists = !fs.fail();
	fs.close();
	return exists;
}

File SD_t::open(char* filename, uint8_t mode) {
	bool create = (mode & O_CREAT);
	mode &= ~O_CREAT;

	File f;
	if (valid && (create || exists(filename))) {
		f.setOutputPath(filename);
		f.begin(0, mode);
	}
	return f;
}

SD_t SD;