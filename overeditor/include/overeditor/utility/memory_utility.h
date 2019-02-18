#ifndef OVEREDITOR_MEMORY_UTILITY_H
#define OVEREDITOR_MEMORY_UTILITY_H

#include <plog/Log.h>

#define BYTES_TO_GIB(bytes) (float) bytes / 1073741824
#define FORMAT_BYTES_AS_GIB(bytes) BYTES_TO_GIB(bytes) << " GiB"

template<typename T>
void printMemory(const T &value) {
    auto addr = &value;

    auto *bytePtr = (uint8_t *) addr;
    std::stringstream ss;
    auto bytePtrEnd = bytePtr + sizeof(T);
    LOG_INFO << "Object resides at " << addr << " -> " << (bytePtrEnd - 1) << "(" << sizeof(T) << " bytes, "
             << typeid(T).name() << ")";
    ss << std::hex;
    do {
        ss << *(bytePtr++);
    } while (bytePtr != bytePtrEnd);
    auto str = ss.str();
    LOG_INFO << "Memory is \"" << str << "\"";
}

template<typename T>
void printTypeMemory() {
    LOG_INFO << "Type " << typeid(T).name() << " uses " << sizeof(T) << " bytes";
}

#endif
