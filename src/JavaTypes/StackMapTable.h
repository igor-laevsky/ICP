///
/// Class which stores stack map table for the method.
///

#ifndef ICP_STACKMAPTABLE_H
#define ICP_STACKMAPTABLE_H

#include "Bytecode/BytecodeFwd.h"
#include "JavaTypes/StackFrame.h"

namespace JavaTypes {

class StackMapTable {
public:
  // Iterates over all stack maps in the method in order of their bci's.
  class Iterator {
  public:
    Iterator() = default;
    Iterator(const StackMapTable &Parent);

    // Expensive to copy
    Iterator(const Iterator &) = delete;
    Iterator &operator=(const Iterator &) = delete;

    // But fine to remove
    Iterator(Iterator &&) = default;
    Iterator &operator=(Iterator &&) = default;

    Bytecode::BciType getBci() const;

    Iterator &operator++();

    const StackFrame &operator*() const;

    bool operator==(const Iterator &Other) const;

  private:
    std::optional<StackFrame> CurFrame;
  };

public:
  StackMapTable(const std::string &Signature);

  bool hasBci(Bytecode::BciType Bci) const;

  Iterator begin() const { return Iterator(InitialFrame); }
  Iterator end() const { return Iterator(); }

private:
  struct Action {
    Bytecode::BciType Bci;

    enum {
      SAME, APPEND, FULL
    } Action;

    StackFrame Data;
  };

private:
  const std::vector<Action> &getActions() const { return Actions; }
  const StackFrame &getInitialFrame() const { return InitialFrame; }

private:
  StackFrame InitialFrame;

  std::vector<Action> Actions;
};

}

#endif //ICP_STACKMAPTABLE_H
