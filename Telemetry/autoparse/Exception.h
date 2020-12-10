#pragma once

#include <exception>
#include <stdio.h>

struct InvalidDatatypeException : virtual public std::exception {
	const char* const EXPECTED;
	InvalidDatatypeException(const char* const EXPECTED) : EXPECTED(EXPECTED) {}
	const char* what() const throw () {
		puts("Invalid Datatype Exception");
		printf("Expected Datatype: %s\n", EXPECTED);
		return "Invalid Datatype Exception";
	}
};

struct InvalidParameterException : virtual public std::exception {
	const char* const TOKEN;
	InvalidParameterException(const char* const TOKEN) : TOKEN(TOKEN) {}
	const char* what() const throw () {
		puts("Invalid Parameter Exception");
		printf("Unable to parse parameter for: %s\n", TOKEN);
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
		puts("Reassignment Exception");
		printf("Cannot assign two values to the same parameter: %s\n", TOKEN);
		printf("Current value: %s\n", CURRENT);
		return "Reassignment Exception";
	}
};

struct SyntaxException : virtual public std::exception {
	const char TOKEN;
	SyntaxException(const char TOKEN) : TOKEN(TOKEN) {}
	inline const char* what() const throw () {
		puts("Syntax Exception");
		printf("Expected Token: %c\n", TOKEN);
		return "Syntax Exception";
	}
};

struct UnmappedFlagException : virtual public std::exception {
	const char* const SET;
	UnmappedFlagException(const char* const SET) : SET(SET) {}
	inline const char* what() const throw () {
		puts("Unmapped Flag Exception");
		printf("Could not find any matching FlagSet: %s\n", SET);
		return "Unmapped Flag Exception";
	}
};