//
// Constant pool implementation
//

#include "ConstantPool.h"

using namespace JavaTypes;

bool ConstantPool::verify(std::string &ErrorMessage) const {
  for (IndexType i = 0; i < numRecords(); ++i) {
    if (getRecordTable()[i] == nullptr) {
      ErrorMessage = "Unallocated record at index " + std::to_string(i + 1);
      return false;
    }
    if (!getRecordTable()[i]->isValid()) {
      ErrorMessage = "Invalid record at index " + std::to_string(i + 1);
      return false;
    }
  }

  return true;
}

bool ConstantPool::verify() const {
  std::string Temp;
  return verify(Temp);
}

void ConstantPool::print(std::ostream &Out) const {
  for (IndexType Idx = 1; Idx <= numRecords(); ++Idx) {
    Out << "#" << Idx << " = ";
    get(Idx).print(Out);
  }
}
