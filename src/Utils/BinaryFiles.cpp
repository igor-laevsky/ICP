#include <algorithm>
#include <cassert>
#include <istream>

#include "BinaryFiles.h"

// Converts number from big endian to whatever native endian is.
static uint64_t bigToNativeEndian(const uint8_t Input[], std::size_t Length) {
  assert(Length >= 1 && Length <= 8 && "Unexpected length");

  uint64_t Res = 0;
  for (std::size_t i = 0; i < Length; ++i) {
    Res |= Input[i] << ((Length - i - 1) * 8);
  }
  return Res;
}

static void readByteArray(uint8_t Output[], std::size_t Length,
                          std::istream &Input) {
  assert(Length >= 1 && Length <= 8 && "Unexpected length");

  Input.read(reinterpret_cast<char*>(Output), Length);
  if (Input.fail() || static_cast<std::size_t>(Input.gcount()) != Length)
    throw Utils::ReadError();
}

uint8_t Utils::BigEndianReading::readByte(std::istream& Input) {
  uint8_t Res = 0;
  readByteArray(&Res, 1, Input);
  return Res;
}

uint16_t Utils::BigEndianReading::readHalf(std::istream& Input) {
  uint8_t Res[2] = {0};
  readByteArray(Res, 2, Input);
  return static_cast<uint16_t>(bigToNativeEndian(Res, 2));
}

uint32_t Utils::BigEndianReading::readWord(std::istream& Input) {
  uint8_t Res[4] = {0};
  readByteArray(Res, 4, Input);
  return static_cast<uint32_t>(bigToNativeEndian(Res, 4));
}

uint64_t Utils::BigEndianReading::readDoubleWord(std::istream& Input) {
  uint8_t Res[8] = {0};
  readByteArray(Res, 8, Input);
  return bigToNativeEndian(Res, 8);
}
