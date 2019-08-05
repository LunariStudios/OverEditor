#ifndef OVEREDITOR_STRING_UTILITY_H
#define OVEREDITOR_STRING_UTILITY_H
#define INDENTATION(level) std::string(level, ' ')
#define EMPTY_STRING  ""
#define BYTE_STRING(var) (int) var << " (" << std::hex << std::showbase << (int) var << std::noshowbase << std::dec <<")"

#endif
