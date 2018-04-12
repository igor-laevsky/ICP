///
/// Utility functions for working with binary files.
///

#ifndef ICP_BINARYFILES_H
#define ICP_BINARYFILES_H

#include <cstdint>
#include <istream>
#include <iostream>

namespace Utils {

class ReadError: std::exception {};

namespace BigEndianReading {

// We rely on this invariant
static_assert(std::is_same_v<unsigned char, uint8_t>);

// All three functions read designated number of bytes from the big endian
// input stream and convert them to a native ending format.
// \returns Value encoded with an ending native to the current platform.
// \throws ReadError In case of reading errors.
uint8_t  readByte(std::istream &Input);
uint16_t readHalf(std::istream &Input);
uint32_t readWord(std::istream &Input);
uint64_t readDoubleWord(std::istream &Input);

}

template<class T, class _X = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr bool isUint8(T Val) {
  return Val >= std::numeric_limits<uint8_t>::min() &&
         Val <= std::numeric_limits<uint8_t>::max();
};

template<class T, class X = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr bool isUint16(T Val) {
  return Val >= std::numeric_limits<uint16_t>::min() &&
         Val <= std::numeric_limits<uint16_t>::max();
};

}

#endif //ICP_BINARYFILES_H
