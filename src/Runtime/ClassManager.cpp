///
/// Class manager implementation
///

#include "ClassManager.h"

#include "JavaTypes/JavaClass.h"

using namespace Runtime;

ClassManager &Runtime::getClassManager() {
  static ClassManager CM;
  return CM;
}

void ClassManager::registerClass(ClassObject &CO) {
  Classes[CO.getClass().getClassName()] = &CO;
}

ClassObject &ClassManager::getClassObject(const Utf8String &Name) {
  assert(Classes.count(Name) == 1);
  assert(Classes.at(Name) != nullptr);

  return *Classes.at(Name);
}

void ClassManager::reset() {
  Classes.clear();
}
