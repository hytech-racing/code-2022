#pragma once

#include <exception>
#include <stdio.h>

struct AutoParseException : virtual public std::exception {
	const char* const ERROR;
	AutoParseException(const char* const ERROR) : ERROR(ERROR) {}
	const char* what() const throw () {
		fputs("Auto Parse Exception", stderr);
		fprintf(stderr, "Unable to generate parser configuration (%s)\n", ERROR);
		return "Auto Parse Exception";
	}
};

struct InvalidDatatypeException : virtual public std::exception {
	const char* const EXPECTED;
	InvalidDatatypeException(const char* const EXPECTED) : EXPECTED(EXPECTED) {}
	const char* what() const throw () {
		fputs("Invalid Datatype Exception", stderr);
		fprintf(stderr, "Expected Datatype: %s\n", EXPECTED);
		return "Invalid Datatype Exception";
	}
};

struct InvalidParameterException : virtual public std::exception {
	const char* const TOKEN;
	InvalidParameterException(const char* const TOKEN) : TOKEN(TOKEN) {}
	const char* what() const throw () {
		fputs("Invalid Parameter Exception", stderr);
		fprintf(stderr, "Unable to parse parameter for: %s\n", TOKEN);
		return "Invalid Parameter Exception";
	}
};

struct ReassignmentException : virtual public std::exception {
	const char* const TOKEN;
	char* const CURRENT;
	ReassignmentException(const char* const TOKEN, char* const CURRENT)
		: TOKEN(TOKEN), CURRENT(CURRENT) {}
	ReassignmentException(const char* const TOKEN, const int current)
		: TOKEN(TOKEN), CURRENT(new char [8]) {
			sprintf(CURRENT, "%d", current);
		}
	const char* what() const throw () {
		fputs("Reassignment Exception", stderr);
		fprintf(stderr, "Cannot assign two values to the same parameter: %s\n", TOKEN);
		fprintf(stderr, "Current value: %s\n", CURRENT);
		return "Reassignment Exception";
	}
};

struct SyntaxException : virtual public std::exception {
	const char TOKEN;
	SyntaxException(const char TOKEN) : TOKEN(TOKEN) {}
	inline const char* what() const throw () {
		fputs("Syntax Exception", stderr);
		fprintf(stderr, "Expected Token: %c\n", TOKEN);
		return "Syntax Exception";
	}
};

struct UnmappedFlagException : virtual public std::exception {
	const char* const SET;
	UnmappedFlagException(const char* const SET) : SET(SET) {}
	inline const char* what() const throw () {
		fputs("Unmapped Flag Exception", stderr);
		fprintf(stderr, "Could not find any matching Flagset: %s\n", SET);
		return "Unmapped Flag Exception";
	}
};