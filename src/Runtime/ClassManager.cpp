///
/// Class manager implementation
///

#include "ClassManager.h"

#include "JavaTypes/JavaClass.h"

using namespace Runtime;

OldClassManager &Runtime::getClassManager() {
  static OldClassManager CM;
  return CM;
}

void OldClassManager::registerClass(ClassObject &CO) {
  Classes[CO.getClass().getClassName()] = &CO;
}

ClassObject &OldClassManager::getClassObject(const Utf8String &Name) {
  assert(Classes.count(Name) == 1);
  assert(Classes.at(Name) != nullptr);

  return *Classes.at(Name);
}

void OldClassManager::reset() {
  Classes.clear();
}
