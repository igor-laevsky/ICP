///
/// Runtime representation of the various java values. This are runtime values
/// of the static types used in verifier.
///

#ifndef ICP_VALUE_H
#define ICP_VALUE_H

#include <cstdint>
#include <memory>
#include <optional>
#include <variant>

#include "JavaTypes/Type.h"

namespace SlowInterpreter {

/// Forward declarations of the entities managed by the GC
///
class Object;
class ClassObject;
class InstanceObject;
class ArrayObject;

/// Runtime data types
///
// No direct support of booleans, but we may choose to optimize them later
using JavaBool = int8_t;
using JavaByte = int8_t;
using JavaChar = uint16_t;
using JavaShort = int16_t;
using JavaInt = int32_t;
using JavaLong = int64_t;
using JavaFloat = float;
using JavaDouble = double;
// Someday this will become GC managed pointer.
using JavaRef = Object*;

// Metaprogramming magic to determine stack type from the given type
template<class T> struct promote_to_stack {};
template<> struct promote_to_stack<JavaByte>   { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaChar>   { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaShort>  { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaInt>    { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaLong>   { using Result = JavaLong;   };
template<> struct promote_to_stack<JavaFloat>  { using Result = JavaFloat;  };
template<> struct promote_to_stack<JavaDouble> { using Result = JavaDouble; };
template<> struct promote_to_stack<JavaRef>    { using Result = JavaRef;    };

template<class T>
using promote_to_stack_t = typename promote_to_stack<T>::Result;

/// Type erased representation for one of the runtime data types.
/// Internaly it promotes all types shorter than 4 bytes into integer. This is
/// to better aprozimate JVM specification.
class Value final {
public:
  class BadAccess: public std::exception { };

public:
  Value() = default;

  /// Creates value. Intention here is to force user to specify the contained
  /// type instead of relying on argument inference for the constructor.
  /// Two overloads are required in order to disable type deduction and retain
  /// move semantics.
  template<class T>
  static constexpr Value create(const std::remove_reference_t<T>& Val) {
    return Value(Val);
  }
  template<class T>
  static constexpr Value create(std::remove_reference_t<T>&& Val) {
    return Value(std::move(Val));
  }

  /// Creates value from plain collection of bytes. Should be used to
  /// efficiently store arrays and class fields.
  /// Caller is responsible for Mem having enough space!
  /// \param T Type which is stored in the memory. Will be promoted into int.
  /// \param Mem Pointer to the array of bytes
  static Value fromMemory(
      const JavaTypes::Type &T, const uint8_t *Mem);

  /// Saves this value to memory. Should be used to effeciently store arrays
  /// and class fields.
  /// Caller is responsible for Mem having enough space!
  /// \throws BadAccess if trying to save a value of the wrong type
  static void toMemory(
      uint8_t *Mem, const Value &V, const JavaTypes::Type &T);

  /// Bunch of type safe accessors.
  /// \throws BadAccess sometimes
  ///

  template<class T>
  constexpr std::optional<T> getAsOrNull() const {
    using PromotedT = promote_to_stack_t<T>;

    if (!std::holds_alternative<PromotedT>(data()))
      return std::nullopt;
    return std::get<PromotedT>(data());
  }

  template<class T>
  constexpr T getAs() const {
    using PromotedT = promote_to_stack_t<T>;

    if (!std::holds_alternative<PromotedT>(data()))
      throw BadAccess();
    return std::get<PromotedT>(data());
  }

  template<class T>
  constexpr bool isA() const {
    using PromotedT = promote_to_stack_t<T>;

    return std::holds_alternative<PromotedT>(data());
  }

  bool operator==(const Value &Other) const {
    return Data == Other.Data;
  }

private:
  // Some trickery to not overload copy and move constructors
  template<class T, class X = std::enable_if_t<!std::is_base_of_v<Value, T>>>
  explicit constexpr Value(T&& Data): Data(Data) {}

  constexpr const auto &data() const { return Data; }

private:
  std::variant<JavaInt, JavaLong, JavaFloat, JavaDouble, JavaRef> Data;
};

}

#endif //ICP_VALUE_H
