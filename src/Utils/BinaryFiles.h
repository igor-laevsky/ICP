//
// Utility functions for working with binary files.
//

#ifndef ICP_BINARYFILES_H
#define ICP_BINARYFILES_H

#include <cstdint>
#include <ios>

namespace Utils {

class ReadError: std::exception {};

namespace BigEndianReading {

// We rely on this invariant
static_assert(std::is_same<unsigned char, uint8_t>::value);

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

#endif //ICP_BINARYFILES_H
