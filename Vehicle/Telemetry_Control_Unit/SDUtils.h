#pragma once

#include <SD.h>
#include <HyTech_FlexCAN.h>
#include <kinetis_flexcan.h>
#include <TimeLib.h>

namespace HT {
namespace SDUtil {

	namespace { uint32_t rtc_ms(); }

	void datetime_callback(uint16_t* date, uint16_t* time) {
		*date = FAT_DATE(year(), month(), day());
		*time = FAT_TIME(hour(), minute(), second());
	}

	bool initFD(File& logfile) {
		if (!SD.begin(BUILTIN_SDCARD)) // Begin Arduino SD API (Teensy 3.5)
			Serial.println("SD card failed or not present");

		char filename[] = "data0000.CSV";
		for (uint8_t i = 0; i < 10000; i++) {
			filename[4] = i / 1000		 + '0';
			filename[5] = i / 100	% 10 + '0';
			filename[6] = i / 10	% 10 + '0';
			filename[7] = i			% 10 + '0';
			if (!SD.exists(filename))
				return logfile = SD.open(filename, O_WRITE | O_CREAT); // Open file for writing
		}
		Serial.println("All possible SD card log filenames are in use - please clean up the SD card");
		return false;
	}

	void write(File& logfile, CAN_message_t& msg) { // Note: This function does not flush data to disk! It will happen when the buffer fills or when the above flush timer fires
		logfile.print(rtc_ms()); logfile.print(',');
		logfile.print(msg.id, HEX); logfile.print(',');
		logfile.print(msg.len);	logfile.print(',');
		for (int i = 0; i < msg.len; i++) {
			if (msg.buf[i] < 0x10)
				logfile.print('0');
			logfile.print(msg.buf[i], HEX);
		}
		logfile.println();
	}

	namespace {
		uint32_t rtc_ms() {
			#define stable_read(reading, src) { do { reading = src; } while (reading != src); }

			uint32_t sec,us;
			stable_read(sec, RTC_TSR);
			stable_read(us, RTC_TPR);

			us = (us * 15625 + 256) / 512; // Round crystal count (simplify to fix overflow: 32.768kHz = 15625 / 512)
			if(us < 100) // If us near 0, may have just incremented seconds -- refetch
				stable_read(sec, RTC_TSR);

			return (sec * 1000) + (us / 1000);
			#undef stable_read
		}
	}
}
}