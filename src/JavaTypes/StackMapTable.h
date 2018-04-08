///
/// Representation of the stack map table for the method.
/// User should first create "StackMapTableBuilder", populate it with the
/// desired building instructions and then create new "StackMapTable" based
/// on the set of method's initial arguments. This is designed in order to
/// better fit the JVM view of the stack maps.
///

#ifndef ICP_STACKMAPTABLE_H
#define ICP_STACKMAPTABLE_H

#include "Bytecode/BytecodeFwd.h"
#include "JavaTypes/StackFrame.h"
#include "Bytecode/BciMap.h"

namespace JavaTypes {

class StackMapTable {
public:
  using Container = Bytecode::BciMap<StackFrame>;
  using const_iterator = Container::const_iterator;

public:
  StackMapTable() = default;

  StackMapTable(const StackMapTable &) = delete;
  StackMapTable &operator=(const StackMapTable &) = delete;
  StackMapTable(StackMapTable &&) = default;
  StackMapTable &operator=(StackMapTable &&) = default;

  const_iterator findAtBci(Bytecode::BciType Bci) const {
    return frames().find(Bci);
  }

  const_iterator begin() const { return Frames.begin(); }
  const_iterator end() const { return Frames.end(); }

private:
  // Only called from the StackMapBuilder.
  explicit StackMapTable(Container &&Input): Frames(std::move(Input)) {
    ;
  }

  const Container &frames() const { return Frames; }

private:
  Container Frames;

  friend class StackMapTableBuilder;
};

class StackMapTableBuilder {
public:
  StackMapTableBuilder() = default;

  StackMapTableBuilder(const StackMapTableBuilder &) = delete;
  StackMapTableBuilder &operator=(const StackMapTableBuilder &) = delete;
  StackMapTableBuilder(StackMapTableBuilder &&) = default;
  StackMapTableBuilder &operator=(StackMapTableBuilder &&) = default;

  // Creates stack map table based on the current content of the builder and on
  // the supplied initial locals array. Usually initial locals are generated
  // from the method descriptor.
  StackMapTable createTable(const std::vector<Type> &InitialLocals) const;

  // Constructs next frame by appending some locals into previous one.
  void addAppend(Bytecode::BciType Idx, std::vector<Type> &&Locals);

  // Next frame is the same as previous one.
  void addSame(Bytecode::BciType Idx);

  // Next frame is fully constructed from input arguments.
  void addFull(Bytecode::BciType Idx,
      std::vector<Type> &&Locals, std::vector<Type> &&Stack);

private:
  struct Action {
    enum FrameTypeEnum {
      APPEND, SAME, FULL
    };

    Bytecode::BciType Bci;
    FrameTypeEnum FrameType;
    std::vector<Type> Locals, Stack;

    Action(Bytecode::BciType Bci, FrameTypeEnum FrameType,
           std::vector<Type> &&Locals, std::vector<Type> &&Stack):
        Bci(Bci), FrameType(FrameType), Locals(std::move(Locals)),
        Stack(std::move(Stack)) {
      ; // Empty
    }
  };

  using Container = std::vector<Action>;

  // Store frame with non-expanded types.
  using RawFrame = std::pair<std::vector<Type>, std::vector<Type>>;

private:
  const Container &actions() const { return FrameActions; }
  Container &actions() { return FrameActions; }

  // Checks that 'Idx' is monotonically increasing in relation to the previous
  // frame. Always true if there are no frames.
  bool checkBciMonotonic(Bytecode::BciType Idx);

  // Transforms given frame accoding with the given action. Transformation is
  // performed in-place.
  static void transformFrame(RawFrame &Frame, const Action &Act);

private:
  Container FrameActions;
};

}

#endif //ICP_STACKMAPTABLE_H
