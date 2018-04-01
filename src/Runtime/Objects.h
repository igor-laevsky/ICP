///
/// Defines runtime representation of the java classes. These objects are
/// intended to be GC managed and referenced using the Value.h::JavaRef
///

#ifndef ICP_OBJECTS_H
#define ICP_OBJECTS_H

#include "JavaTypes/JavaTypesFwd.h"
#include "Runtime/Value.h"
#include "Utils/Utf8String.h"
#include "Runtime/FieldStorage.h"

namespace Runtime {

/// Base abstract class for any type of the runtime object.
class Object {
public:
  class BadAccess: public std::exception { };

public:
  virtual ~Object() = default;

  /// Type safe accessors. These are more or less transparent applications
  /// of the default RTTI and only needed to prevent dynamic_casts from
  /// spreading across the code.
  /// \throws BasAccess when necesary

  template<class T> bool isA() const noexcept {
    return dynamic_cast<const T*>(this) != nullptr;
  }

  template<class T> const T& getAs() const {
    const T *Ret = dynamic_cast<const T*>(this);
    if (Ret == nullptr)
      throw BadAccess();
    return *Ret;
  }
  // Allows user to not care about const qualifier
  template<class T> T& getAs() {
    return const_cast<T&>(const_cast<const Object*>(this)->getAs<T>());
  }

  template<class T> const T* getAsOrNull() const noexcept {
    return dynamic_cast<const T*>(this);
  }
  template<class T> T* getAsOrNull() noexcept {
    return dynamic_cast<T*>(this);
  }

protected:
  Object() = default;
};

/// Class which represents the loaded java class itself.
class ClassObject final: public Object {
public:
  // No copies
  ClassObject(const ClassObject &) = delete;
  ClassObject &operator=(const ClassObject &) = delete;
  // No moves
  ClassObject(ClassObject &&) = delete;
  ClassObject &operator=(ClassObject &&) = delete;

  /// Create the class and zero-initializes it's static fields
  explicit ClassObject(const JavaTypes::JavaClass &Class) :
    Class(Class),
    Fields(Class, /*is_static**/true) {
    ;
  }

  /// Get static field from this class.
  /// \throws UnrecognizedField If no field was found.
  Value getField(const Utf8String &Name) const {
    return Fields.getField(Name);
  }

  /// Set static field or throw an exception if no such field is found.
  /// \throws UnrecognizedField If no field was found.
  void setField(const Utf8String &Name, const Value &V) {
    return Fields.setField(Name, V);
  }

  const JavaTypes::JavaClass &getClass() const { return Class; }

  // Resolve the method
  const JavaTypes::JavaMethod *getMethod(const Utf8String &Name) const;

private:
  const JavaTypes::JavaClass &Class;
  FieldStorage Fields;
};

/// Class which represents instance of the java class (ClassObject)
class InstanceObject final: public Object {
public:
  // TODO: This should return gc managed pointer someday
  static InstanceObject *create(ClassObject &ClassObj);

  /// Get instance field from this class.
  /// \throws UnrecognizedField If no field was found.
  Value getField(const Utf8String &Name) const {
    return Fields.getField(Name);
  }

  /// Set instance field or throw an exception if no such field is found.
  /// \throws UnrecognizedField If no field was found.
  void setField(const Utf8String &Name, const Value &V) {
    Fields.setField(Name, V);
  }

  ClassObject &getClassObj() { return ClassObj; }
  const ClassObject &getClassObj() const { return ClassObj; }

  const JavaTypes::JavaClass &getClass() const { return ClassObj.getClass(); }

private:
  explicit InstanceObject(ClassObject &ClassObj):
    ClassObj(ClassObj),
    Fields(ClassObj.getClass(), /*is_static*/false) {
    ;
  }

private:
  ClassObject &ClassObj;
  FieldStorage Fields;
};

}

#endif //ICP_OBJECTS_H
