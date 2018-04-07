///
/// Utilities intended to help with the bytecode instruction definitions.
///

#ifndef ICP_INSTRUCTIONUTILS_H
#define ICP_INSTRUCTIONUTILS_H

#include "Bytecode.h"

namespace Bytecode {

// Utility class for instruction consisting of a single byte.
template<class ConcreteType>
class NoIndex : public VisitableInstruction<ConcreteType> {
public:
  static constexpr uint8_t Length = 1;

  void print(std::ostream &Out) const override {
    Out << ConcreteType::Name << "\n";
  }

private:
  // Adhere to the common interface used in the create functions.
  explicit NoIndex(ContainerIterator It) {
    (void) It;
  }

  // Allow calling constructor from the create functions.
  friend class Instruction;
};

// Utility class for three byte instructions. First byte is opcode,
// second two represent constant pool index.
template<class ConcreteType, class T = IdxType>
class SingleIndex : public VisitableInstruction<ConcreteType> {
public:
  static constexpr uint8_t Length = 1 + sizeof(T);
  static_assert(Length == 2 || Length == 3);

public:
  T getIdx() const {
    return Idx;
  }

  void print(std::ostream &Out) const override {
    Out << ConcreteType::Name << " #" << getIdx() << "\n";
  }

private:
  static T parseIdx(ContainerIterator It) {
    if (Length == 2) {
      return *(It + 1);
    } else if (Length == 3) {
      return (*(It + 1) << 8) | *(It + 2);
    } else {
      assert(false); // unhandled length
    }
  }

  explicit SingleIndex(ContainerIterator It) :
      Idx(parseIdx(It)) {
    ; // Empty
  }

  // Allow calling constructor from the Instruction::create functions
  friend class Instruction;

private:
  const T Idx;
};

// Same as SingleIndex but has signed index type.
template<class ConcreteType>
using OffsetIndex = SingleIndex<ConcreteType, BciOffsetType>;

// Same as SingleIndex but has one byte index instead.
template<class ConcreteType>
using ByteIndex = SingleIndex<ConcreteType, ByteIdxType>;


// Determine index type of the given instruction. We can have different index
// types - signed, unsigend, wide. Results in a substitution failure if at least
// one of the instructions is not indexed.
template<class... Ts>
using InstIdxType = std::common_type_t<
    std::result_of_t<decltype(&Ts::getIdx)(Ts)>...>;

// Determine value type of the given instruction. Substitution failure if at
// least one of the instructions is not indexed.
template<class... Ts>
using InstValType = std::common_type_t<decltype(Ts::Val)...>;


// Better move this into utility module but so far it's the only place
// where it's needed.
template<class Arg, class... Args>
inline constexpr bool contains =
    std::disjunction<std::is_same<Arg, Args>...>::value;
template<class Arg>
inline constexpr bool contains<Arg> = false;


// Wraps number of instruction with values. It is used to simplify instruction
// visitor which can now accept a single wrapper instead of a set of almost
// identical instructions.
template<class... Ts>
class ValueInstWrapper {
private:
  using ThisValType = InstValType<Ts...>;

public:
  template<
      class InstT,
      // Check that we declared this instruction.
      class X = std::enable_if_t<contains<InstT, Ts...>>,
      // Check that instruction has the expected value type.
      class Y = std::enable_if_t<
          std::is_same_v<InstValType<InstT>, ThisValType>>>
  constexpr ValueInstWrapper(const InstT &Inst) noexcept:
      Inst(Inst),
      Val(InstT::Val) {
    ;
  }

  // Get this instruction's value from it's index. This is used to uniformly
  // handle such instructions as 'aload' and 'aload_n'.
  struct from_idx_t {
    explicit from_idx_t() = default;
  };

  static constexpr from_idx_t from_idx;

  template<
      class InstT,
      // Check that index of this instruction has the same type as values of
      // all the other instructions.
      class Y = std::enable_if_t<std::is_same_v<InstIdxType<InstT>, ThisValType>>>
  constexpr ValueInstWrapper(from_idx_t, const InstT &Inst) noexcept:
      Inst(Inst),
      Val(Inst.getIdx()) {
    ;
  }

  constexpr ThisValType getVal() const { return Val; }

  constexpr const Instruction &getInst() const { return Inst; }

protected:
  const Instruction &Inst;
  const ThisValType Val = 0;
};

// Wraps number of indexed instructions and provides uniform access to their
// index. Mainly needed for the 'ValueIdxWrapper'.
template<class... Ts>
class IdxInstWrapper {
private:
  using ThisIdxType = InstIdxType<Ts...>;

public:
  template<
      class InstT,
      class X = std::enable_if_t<contains<InstT, Ts...>>>
  constexpr IdxInstWrapper(const InstT &Inst):
      Idx(Inst.getIdx()) {
    ;
  }

  ThisIdxType getIdx() const { return Idx; }

private:
  const ThisIdxType Idx;
};

// Wraps instructions which are both indexed and have values. Main motivation is
// to provide access to the instruction indexes while keeping them wrapped into
// value wrapper.
template<class... Ts>
class ValueIdxWrapper :
    public ValueInstWrapper<Ts...>, public IdxInstWrapper<Ts...> {
public:
  template<class InstT, class X = std::enable_if_t<contains<InstT, Ts...>>>
  ValueIdxWrapper(const InstT &Inst):
      ValueInstWrapper<Ts...>(Inst),
      IdxInstWrapper<Ts...>(Inst) {
    ;
  }
};

}

#endif //ICP_INSTRUCTIONUTILS_H
