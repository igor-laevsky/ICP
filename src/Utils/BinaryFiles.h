//
// Utility functions for working with binary files.
//

#ifndef IJVM_BINARYFILES_H
#define IJVM_BINARYFILES_H

#include <cstdint>
#include <ios>

namespace Utils {

class ReadError: std::exception {};

namespace BigEndianReading {

// All three functions read designated number of bytes from the big endian
// input stream and convert them to a native ending format.
// \returns Value encoded with an ending native to the current platform.
// \throws ReadError In case of reading errors.
uint8_t  readByte(std::istream &Input);
uint16_t readHalf(std::istream &Input);
uint32_t readWord(std::istream &Input);
uint64_t readDoubleWord(std::istream &Input);

}

}

#endif //IJVM_BINARYFILES_H
