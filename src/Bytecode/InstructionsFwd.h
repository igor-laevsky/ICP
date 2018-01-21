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

using Container = std::vector<uint8_t>;
using ContainerIterator = Container::const_iterator;

class Instruction;

template<class T> class NoIndex;
template<class T> class SingleIndex;

class aload;
class aload_0;
class invokespecial;
class java_return;
class iconst_m1;
class iconst_0;
class iconst_1;
class iconst_2;
class iconst_3;
class iconst_4;
class iconst_5;
class iconst_val;
class dconst_0;
class dconst_1;
class dconst_val;
class ireturn;
class dreturn;
class putstatic;
class getstatic;

class if_icmp_op;
class if_icmpeq;
class if_icmpne;
class if_icmplt;
class if_icmpge;
class if_icmpgt;
class if_icmple;

}

#endif //ICP_BYTECODEINSTRUCTIONSFWD_H
