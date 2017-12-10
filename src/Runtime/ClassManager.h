///
/// Class which manages all java classes. This is very preliminary and
/// doesn't support any interesting features.
///

#ifndef ICP_CLASSMANAGER_H
#define ICP_CLASSMANAGER_H

#include "Runtime/Objects.h"

#include <map>

namespace Runtime {

class ClassManager {
public:
  friend ClassManager &getClassManager();

  ClassManager(const ClassManager&) = delete;
  ClassManager &operator=(const ClassManager&) = delete;

  void registerClass(ClassObject &CO);

  ClassObject &getClassObject(const Utf8String &Name);

private:
  ClassManager() = default;

private:
  std::map<Utf8String, ClassObject*> Classes;

};

ClassManager &getClassManager();

}

#endif //ICP_CLASSMANAGER_H
