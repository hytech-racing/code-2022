#pragma once

enum class ParseType { None, Variable, Class, Flag };

constexpr const char* const S_COMMENT = "//";
constexpr const char* const M_COMMENT_START = "/*";
constexpr const char* const M_COMMENT_CLOSE = "*/";

constexpr const char* const PARSE = "Parse";
constexpr const char* const CLASS = "class";
constexpr const char* const FLAG = "flag";

constexpr const char* const ID = "ID";
constexpr const char* const PREFIX = "Prefix";
constexpr const char* const CUSTOM = "Custom";
constexpr const char* const NAME = "Name";
constexpr const char* const GETTER = "Getter";
constexpr const char* const HEX = "Hex";
constexpr const char* const UNIT = "Unit";
constexpr const char* const SCALE = "Scale";
constexpr const char* const PRECISION = "Precision";
constexpr const char* const FLAGLIST = "Flaglist";
constexpr const char* const FLAGSET = "Flagset";
constexpr const char* const FLAGPREFIX = "Flagprefix";
constexpr const char* const SPARSE = "Sparse";

constexpr const char* const ID_PREFIX = "ID_";
constexpr const char* const GET_PREFIX = "get_";

inline size_t constexpr ct_strlen(const char* str) {
	size_t len = 0;
	while (*str++) ++len;
	return len;
}