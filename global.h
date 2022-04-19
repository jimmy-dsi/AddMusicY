#ifndef GLOBAL_H
#define GLOBAL_H

#include "byte.h"
#include <string>

// global constants
const unsigned DATA_CAP = 8977;
const unsigned START_OFFSET = 0x0;
const unsigned short ARAM_START = 0xDC7F;

const byte NOTE_TABLE[7] = { 9, 11, 0, 2, 4, 5, 7 };
const std::string NOTE_STRINGS[12] = { "c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b" };
//

#endif // GLOBAL_H
