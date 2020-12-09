#pragma once

enum class ParseType { None, Variable, Class, Flag };

constexpr char S_COMMENT [] = "//";
constexpr char M_COMMENT_START [] = "/*";
constexpr char M_COMMENT_CLOSE [] = "*/";

constexpr char PARSE [] = "parse";
constexpr char CLASS [] = "class";
constexpr char FLAG [] = "flag";

constexpr char ID [] = "ID";
constexpr char PREFIX [] = "PREFIX";
constexpr char CUSTOM [] = "CUSTOM";

constexpr char NAME [] = "NAME";
constexpr char GETTER [] = "GETTER";
constexpr char HEX [] = "HEX";
constexpr char UNIT [] = "UNIT";
constexpr char SCALE [] = "SCALE";
constexpr char FLAGLIST [] = "FLAGLIST";
constexpr char FLAGSET [] = "FLAGSET";
constexpr char FLAGPREFIX [] = "FLAGPREFIX";

constexpr char ID_PREFIX [] = "ID_";

inline size_t constexpr ct_strlen(const char* str) {
	size_t len = 0;
	while (*str++) ++len;
	return len;
}