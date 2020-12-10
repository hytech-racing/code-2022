#pragma once

enum class ParseType { None, Variable, Class, Flag };

constexpr char S_COMMENT [] = "//";
constexpr char M_COMMENT_START [] = "/*";
constexpr char M_COMMENT_CLOSE [] = "*/";

constexpr char PARSE [] = "Parse";
constexpr char CLASS [] = "class";
constexpr char FLAG [] = "flag";

constexpr char ID [] = "ID";
constexpr char PREFIX [] = "Prefix";
constexpr char CUSTOM [] = "Custom";

constexpr char NAME [] = "Name";
constexpr char GETTER [] = "Getter";
constexpr char HEX [] = "Hex";
constexpr char UNIT [] = "Unit";
constexpr char SCALE [] = "Scale";
constexpr char PRECISION [] = "Precision";
constexpr char FLAGLIST [] = "Flaglist";
constexpr char FLAGSET [] = "Flagset";
constexpr char FLAGPREFIX [] = "Flagprefix";

constexpr char ID_PREFIX [] = "ID_";

inline size_t constexpr ct_strlen(const char* str) {
	size_t len = 0;
	while (*str++) ++len;
	return len;
}