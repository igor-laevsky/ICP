///
/// Implementation for the runtime java object representation
///

#include "Objects.h"

#include "JavaTypes/JavaClass.h"

using namespace Runtime;
using namespace JavaTypes;

const JavaMethod *ClassObject::getMethod(const Utf8String &Name) const {
  return getClass().getMethod(Name);
}

InstanceObject *InstanceObject::create(ClassObject &Class) {
  return new InstanceObject(Class);
}

InstanceObject::InstanceObject(ClassObject &ClassObj): ClassObj(ClassObj) {
  (void)this->ClassObj;
}
