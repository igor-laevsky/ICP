///
/// Constant pool implementation.
///

#include "ConstantPool.h"

using namespace JavaTypes;

void ConstantPool::print(std::ostream &Out) const {
  for (IndexType Idx = 1; Idx <= numRecords(); ++Idx) {
    Out << "#" << Idx << " = ";
    get(Idx).print(Out);
  }
}
