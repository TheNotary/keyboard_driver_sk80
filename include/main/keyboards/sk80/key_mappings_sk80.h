#pragma once
#include <unordered_map>
#include <string>


namespace sk80 {

    const std::unordered_map<std::string, char> keyname_keyid_mappings = {
        {
            {"esc", 1},
            {"f1" , 2},
            {"f2" , 3},
            {"f3" , 4},
            {"f4" , 5},
            {"f5" , 6},
            {"f6" , 7},
            {"f7" , 8},
            {"f8" , 9},
            {"f9" , 10},
            {"f10", 11},
            {"f11", 12},
            {"f12", 13},

            {"`", 19},
            {"1", 20},
            {"2", 21},
            {"3", 22},
            {"4", 23},
            {"5", 24},
            {"6", 25},
            {"7", 26},
            {"8", 27},
            {"9", 28},
            {"0", 29},
            {"-", 30},
            {"=", 31},
            {"backspace", 2},

            {"tab", 37},
            {"q", 38},
            {"w", 39},
            {"e", 40},
            {"r", 41},
            {"t", 42},
            {"y", 43},
            {"u", 44},
            {"i", 45},
            {"o", 46},
            {"p", 47},
            {"[", 48},
            {"]", 49},

            {"caps", 55},
            {"a", 56},
            {"s", 57},
            {"d", 58},
            {"f", 59},
            {"g", 60},
            {"h", 61},
            {"j", 62},
            {"k", 63},
            {"l", 64},
            {";", 65},
            {"'", 66},
            {"\\", 67},


            {"lshift", 73},
            {"z", 74},
            {"x", 75},
            {"c", 76},
            {"v", 77},
            {"b", 78},
            {"n", 79},
            {"m", 80},
            {",", 81},
            {".", 82},
            {"/", 83},
            {"shift", 84},
            {"enter", 85},
            {"lctrl", 91},
            {"meta", 92},
            {"lalt", 93},
            {"space", 94},
            {"ralt", 95},
            {"fn", 96},
            {"rctrl", 98},

            {"left", 99},
            {"down", 100},
            {"up", 101},
            {"right", 102},
            {"backspace", 103},

            {"insert", 116},
            {"pageup", 118},
            {"delete", 119},
            {"pagedown", 121},

        }
    };

}
