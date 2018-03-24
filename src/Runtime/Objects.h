///
/// Defines runtime representation of the java classes. These objects are
/// intended to be GC managed and referenced using the Value.h::JavaRef
///

#ifndef ICP_OBJECTS_H
#define ICP_OBJECTS_H

#include "JavaTypes/JavaTypesFwd.h"
#include "Value.h"
#include "Utils/Utf8String.h"

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
class ClassObject: public Object {
public:
  class UnrecognizedField: public std::exception { };

public:
  /// Creates class and zero initializes it. So far this ignores java rules of
  /// the class loading and initialization.
  /// This may become private and go in into the separate class manager.
  static JavaRef create(const JavaTypes::JavaClass &Class);

  /// Get static field from this class.
  /// \throws UnrecognizedField If no field was found.
  Value getField(const Utf8String &Name) const;

  /// Set static field or throw an exception if no such field is found.
  /// \throws UnrecognizedField If no field was found.
  void setField(const Utf8String &Name, const Value &V);

  const JavaTypes::JavaClass &getClass() const { return Class; }

  // Resolve the method
  const JavaTypes::JavaMethod *getMethod(const Utf8String &Name) const;

private:
  explicit ClassObject(const JavaTypes::JavaClass &Class);

  const std::vector<uint8_t> &fields() const { return Fields; }
  std::vector<uint8_t> &fields() { return Fields; }

  std::pair<const JavaTypes::JavaField*, std::size_t>
  findFieldAndOffset(const Utf8String &Name) const;

private:
  const JavaTypes::JavaClass &Class;
  std::vector<uint8_t> Fields;

  friend class ClassManager;
};

}

#endif //ICP_OBJECTS_H
