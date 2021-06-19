#pragma once

#include <fstream>
#include <cstring>

#include "Token.h"

#define BUFFER_LENGTH 1024

enum class StopMode { COMMENT, FILE, LINE };

class InputBuffer {
public:
	InputBuffer (const char* filepath, size_t len = BUFFER_LENGTH);
	~InputBuffer();
	char* find(const char c);
	size_t getline();
	bool eat(const char* const token, size_t len);
	bool getToken(char* token);
	void getParam(char* token, const char* const ANNOTATION);
	inline int getInt() { eatspace(); return wrap() ? (int) strtol(current, &current, 10) : 0; };
	
	inline void setStopMode(StopMode mode) { stop = mode; }

	inline bool wrap() { return *current != '\0' || getline(); }
	inline char get() { return wrap() ? *++current : '\0'; }
	inline bool eatspace() { return *current == ' ' ? (bool) ++current : *current == '\0'; }
	inline bool eat(const char c) { eatspace(); wrap(); return *current == c ? (bool) (current += (c != '\0')) : false; }

	inline void skip(StopMode mode) { std::swap(mode, stop); while(getline()); stop = mode; }
	inline const char* lineStart() { return buffer; }
	inline void restartLine() { current = buffer; }
	inline void load(char* stash) { strcpy(buffer, stash); current = buffer; stop = StopMode::LINE; }

	inline void show() { puts(current); }
	inline void showLine(FILE* f = stdin) { fprintf(f, "%s:%d\n%s\n\n", FILEPATH, linenum, buffer); }
private:
	inline bool atStopPoint() {
		if (stop == StopMode::FILE)
			return infile.eof();
		if (stop == StopMode::COMMENT)
			return strstr(buffer, M_COMMENT_CLOSE);
		return true;
	}

	int linenum = 0;
	char* current;
	char* const buffer;
	const char* const FILEPATH;
	const size_t SIZE;
	StopMode stop;
	std::ifstream infile;
};
