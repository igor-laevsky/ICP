//
// Utility functions for working with binary files.
//

#ifndef ICP_BINARYFILES_H
#define ICP_BINARYFILES_H

#include <cstdint>
#include <istream>

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

template<class T, class X = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool is8bit(T Val) {
  return (Val & 0xFF) == Val;
};

template<class T, class X = std::enable_if_t<std::is_integral_v<T>>>
constexpr bool is16bit(T Val) {
  return (Val & 0xFFFF) == Val;
};

}

#endif //ICP_BINARYFILES_H
