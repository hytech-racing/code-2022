#include "InputBuffer.h"

#include <stdio.h>
#include "Exception.h"
#include "Utils.h"

InputBuffer::InputBuffer(const char* filepath, size_t len) : 
	SIZE(len),
	buffer(new char[len]),
	infile(filepath),
	FILEPATH(filepath)
{
	current = buffer;
	*current = '\0';
	stop = StopMode::FILE;
}

InputBuffer::~InputBuffer() {
	delete [] buffer;
	infile.close();
}

char* InputBuffer::find(const char c) {
	do {
		for (current; *current; ++current)
			if (*current == c)
				return current;
	} while(getline());
	return nullptr;
}

size_t InputBuffer::getline() {
	if (atStopPoint())
		return false;

	++linenum;
	current = buffer;

	int i = 0;
	while (i < SIZE) {
		char c = infile.get();
		switch(c) {
			case '\r':	c = infile.get(); // skip this
			case '\n':
				if (i == 0) {
					++linenum;
					break; // ignore empty lines
				}
			case EOF:
				if (i != 0 && buffer[i - 1] == ' ')
					--i;
				buffer[i] = '\0';
				return i;
			case '\t': c = ' ';
			case ' ':
				if (i == 0 || buffer[i - 1] == ' ') // ignore leading / repeated whitespace
					break;
			case '*':
				if (i == 0 && infile.peek() == ' ') // ignore isolated line-start *'s (multi-line comments)
					break;
			default:
				buffer[i++] = c;
		}
	}
	fprintf(stderr, "BUFFER OVERFLOW ERROR - current maximum = %d bytes\n", BUFFER_LENGTH);
	exit(EXIT_FAILURE);
}

bool InputBuffer::eat(const char* token, size_t len) {
	char* text = current;
	while (*token)
		if (*token++ != *text++)
			return false;
	current += len;
	return true;
}

void InputBuffer::getParam(char* token, const char* const ANNOTATION) {
	if (!strempty(token))
		throw ReassignmentException(ANNOTATION, token);	

	char* const start = token;
	eatspace();
	if (!wrap())
		throw InvalidParameterException(ANNOTATION);	

	int parenStack = 1;

	while (*current || getline()) {
		char c = *current;
		if ((parenStack += (c == '(') - (c == ')')) == (c == ','))
			break;
		*token++ = *current++;
	}

	if (token == start)
		throw InvalidParameterException(ANNOTATION);	

	if (*(token - 1) == ' ')
		--token;
 	*token = '\0';
}

bool InputBuffer::getToken(char* token) {
	char* const start = token;
	eatspace();
	if (!wrap()) 
		return false;

	while (isalnum(*current) || *current == '_')
		*token++ = *current++;
	*token = '\0';
	return token != start;
}