#pragma once
#include <ios>
#include "Serial.h"

#define BUILTIN_SDCARD 254

#define O_WRITE std::ios_base::out
#define O_CREAT std::ios_base::trunc

class File : public MockSerial {
public:
	File() : MockSerial(BUILTIN_SDCARD) {}
};

class SD_t {
public:
	bool begin(uint8_t cspin);
	bool exists(char* filename);
	File open(char* filename, uint8_t mode);
private:
	bool valid = false;
};
extern SD_t SD;

namespace SdFile {
	void dateTimeCallback(void (*cb)(uint16_t*, uint16_t*)) {}
}

static inline uint16_t FAT_DATE(uint16_t year, uint8_t month, uint8_t day) {
  return (year - 1980) << 9 | month << 5 | day;
}

static inline uint16_t FAT_TIME(uint8_t hour, uint8_t minute, uint8_t second) {
  return hour << 11 | minute << 5 | second >> 1;
}