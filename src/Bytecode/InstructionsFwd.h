//
// Forward declarations of the bytecode instructions
//

#ifndef ICP_BYTECODEINSTRUCTIONSFWD_H
#define ICP_BYTECODEINSTRUCTIONSFWD_H

namespace Bytecode {

class Instruction;

template<class T> class NoIndex;
template<class T> class SingleIndex;

class aload;
class aload_0;
class invokespecial;
class java_return;
class iconst_0;
class iconst_1;
class iconst_val;
class dconst_0;
class dconst_1;
class dconst_val;
class ireturn;
class dreturn;
class putstatic;
class getstatic;

}

#endif //ICP_BYTECODEINSTRUCTIONSFWD_H
