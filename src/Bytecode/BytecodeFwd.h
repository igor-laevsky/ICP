//
// Forward declarations of the bytecode instructions
//

#ifndef ICP_BYTECODEINSTRUCTIONSFWD_H
#define ICP_BYTECODEINSTRUCTIONSFWD_H

#include <cstdint>
#include <vector>

namespace Bytecode {

using BciType = uint32_t;
using IdxType = uint16_t;
using ByteIdxType = uint8_t;
using BciOffsetType = int16_t;
static_assert(sizeof(IdxType) == sizeof(BciOffsetType), "offsets are encoded as indexes");

using Container = std::vector<uint8_t>;
using ContainerIterator = Container::const_iterator;

template<typename T>
class BciMap;

class Instruction;

template<class T> class NoIndex;
template<class T, class U> class SingleIndex;

#define HANDLE_INSTR_ALL(ClassName) class ClassName;
#define HANDLE_WRAPPER(ClassName) class ClassName;
#include "Instructions.inc"

}

#endif //ICP_BYTECODEINSTRUCTIONSFWD_H
