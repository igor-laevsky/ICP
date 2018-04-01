///
/// Runtime representation of the various java values. This are runtime values
/// of the static types used in verifier.
///

#ifndef ICP_VALUE_H
#define ICP_VALUE_H

#include "JavaTypes/Type.h"
#include "Runtime/RuntimeFwd.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <variant>

namespace Runtime {

/// Type erased representation for one of the runtime data types.
/// Internally it promotes all types shorter than 4 bytes into integer. This is
/// to better approximate JVM specification.
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
    using PromotedT = promote_to_stack_t<T>;
    return Value(static_cast<PromotedT>(Val));
  }
  template<class T>
  static constexpr Value create(std::remove_reference_t<T>&& Val) {
    using PromotedT = promote_to_stack_t<T>;
    return Value(std::move(static_cast<PromotedT>(Val)));
  }

  /// Creates value from plain collection of bytes. Should be used to
  /// efficiently store arrays and class fields.
  /// Reads no more than T's size in bytes.
  /// Caller is responsible for the Mem having enough space.
  /// \param T Type which is stored in the memory. Will be promoted into int.
  /// \param Mem Pointer to the array of bytes
  static Value fromMemory(
      const JavaTypes::Type &T, const uint8_t *Mem);

  /// Saves this value to memory. Should be used to effeciently store arrays
  /// and class fields.
  /// Writes no more than T's size in bytes.
  /// Caller is responsible for the Mem having enough space.
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

  /// Converts this value to it's string representation.
  /// This is intended for the debug purposes, don't use it in any correctness
  /// related applications, i.e tests.
  friend std::string to_string(const Value &V);
  friend std::ostream& operator<<(std::ostream &Out, const Value &V);

private:
  using DataType =
    std::variant<JavaInt, JavaLong, JavaFloat, JavaDouble, JavaRef>;

  // Some trickery to not overload copy and move constructors
  template<class T, class X =
      std::enable_if_t<
          !std::is_base_of_v<Value,
              std::remove_reference_t<T>>>>
  explicit constexpr Value(T&& Data): Data(Data) {}

  constexpr const DataType &data() const { return Data; }

private:
  DataType Data;
};

std::string to_string(const Value &V);
std::ostream& operator<<(std::ostream &Out, const Value &V);

}

#endif //ICP_VALUE_H
