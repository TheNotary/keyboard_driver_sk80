#pragma once
#include <unordered_map>
#include <string>
#include "misc.h"

constexpr auto MESSAGE_LENGTH_RK84 = 65;
constexpr auto BULK_LED_VALUE_MESSAGES_COUNT_RK84 = 3;

extern unsigned char BULK_LED_VALUE_MESSAGES_RK84[][MESSAGE_LENGTH_RK84];
//extern const size_t  BULK_LED_VALUE_MESSAGES_COUNT_RK84;



//short asdf = 0x00;

//std::unordered_map<std::string, short> asfdasdf;

/*
KeyNameKeyIdPair keyname_keyid_mappings_z = {
	{"esc", 1},
	{"f1", 7},
	{"f2", 13},
	{"f3", 19},
	{"f4", 25},
	{"f5", 31},
	{"f6", 37},
	{"f7", 43},
	{"f8", 49},
	{"f9", 55},
	{"f10", 61},
	{"f11", 67},
	{"f12", 73},
	{"print", 79},
	{"pause", 85},
	{"delete", 91},

	{"`", 2},
	{"1", 8},
	{"2", 14},
	{"3", 20},
	{"4", 26},
	{"5", 32},
	{"6", 38},
	{"7", 44},
	{"8", 50},
	{"9", 56},
	{"0", 62},
	{"-", 68},
	{"=", 74},
	{"backspace", 80},
	{"home", 92},

	{"tab", 3},
	{"q", 9},
	{"w", 15},
	{"e", 21},
	{"r", 27},
	{"t", 33},
	{"y", 39},
	{"u", 45},
	{"i", 51},
	{"o", 57},
	{"p", 63},
	{"[", 69},
	{"]", 75},
	{"\\", 81},
	{"end", 93},

	{"caps", 4},
	{"a", 10},
	{"s", 16},
	{"d", 22},
	{"f", 28},
	{"g", 34},
	{"h", 40},
	{"j", 46},
	{"k", 52},
	{"l", 58},
	{";", 64},
	{"'", 70},
	{"enter", 82},
	{"pageup", 94},

	{"lshift", 5},
	{"z", 11},
	{"x", 17},
	{"c", 23},
	{"v", 29},
	{"b", 35},
	{"n", 41},
	{"m", 47},
	{",", 53},
	{".", 59},
	{"/", 65},
	{"rshift", 71},
	{"up", 83},
	{"pagedown", 95},

	{"lctrl", 6},
	{"lmeta", 12},
	{"lalt", 18},
	{"space", 36},
	{"ralt", 54},
	{"fn", 60},
	{"rctrl", 66},
	{"left", 72},
	{"down", 84},
	{"right", 96},
};
*/