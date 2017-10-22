//
// Implementation of the interpreter.
//

#include "Interpreter.h"

#include <cassert>
#include <vector>
#include <cstdint>

using namespace Interpreter;

namespace {

// Represents single interpreter frame.
// All dynamic cast functions are expected to succeed for the methods which
// passed verification.
class InterpreterFrame {
public:
  explicit InterpreterFrame(std::vector<std::any> Locals):
      Locals(std::move(Locals)) {
    ;
  }

  template<class T>
  T getLocal(uint32_t Idx) const {
    assert(Idx < locals().size());
    return std::any_cast<T>(locals()[Idx]);
  }

  template<class T>
  void setLocal(uint32_t Idx, T NewVal) {
    assert(Idx < locals().size());
    locals()[Idx] = std::any(std::move(NewVal));
  }

  template<class T>
  T pop() {
    assert(Stack.size() > 0);

    T Ret = std::any_cast<T>(stack().back());
    stack().pop_back();
    return Ret;
  }

  template<class T>
  void push(T Val) {
    Stack.push_back(std::any(std::move(Val)));
  }

private:
  auto &locals() { return Locals; }
  const auto &locals() const { return Locals; }

  auto &stack() { return Stack; }
  const auto &stack() const { return Stack; }

private:
  std::vector<std::any> Locals;
  std::vector<std::any> Stack;
};

}

std::any Interpreter::Interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<std::any> &InputArguments) {

  (void)Method;
  (void)InputArguments;
  return std::any();
}
