//
// Implementation of the interpreter.
//

#include "Interpreter.h"

#include <cassert>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

#include "Bytecode/InstructionVisitor.h"
#include "JavaTypes/JavaMethod.h"

using namespace Bytecode;
using namespace SlowInterpreter;

namespace {

// Represents single interpreter frame.
// All dynamic cast functions are expected to succeed for the methods which
// passed verification.
class InterpreterFrame final {
public:
  explicit InterpreterFrame(
      std::string FunctionName,
      std::vector<std::any> Locals) noexcept:
    FunctionName(std::move(FunctionName)),
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
  T popSingle() {
    assert(!stack().empty());

    auto Ret = std::any_cast<T>(stack().back());
    stack().pop_back();
    return Ret;
  }
  template<class T>
  T popDouble() {
    popSingle<T>();
    return popSingle<T>();
  }

  // Note: Disable template type deduction. User should explicitly think
  // what type is going to be pushed to stack.
  template<class T>
  void pushSingle(std::common_type_t<T> Val) {
    stack().push_back(std::any(std::move(Val)));
  }
  template<class T>
  void pushDouble(std::common_type_t<T> Val) {
    pushSingle<T>(std::move(Val));
    pushSingle<T>(0);
  }

private:
  auto &locals() { return Locals; }
  const auto &locals() const { return Locals; }

  auto &stack() { return Stack; }
  const auto &stack() const { return Stack; }

private:
  std::string FunctionName;
  std::vector<std::any> Locals;
  std::vector<std::any> Stack;
};

// Represents stack of InterpreterFrames.
// All reasonable invariants (like not exiting function before entering any) are
// enforced via asserts since they were already checked by the class verifier.
class InterpreterStack final {
public:
  void enter_function(
      std::string FunctionName,
      std::vector<std::any> Arguments) {
    stack().emplace_back(std::move(FunctionName), std::move(Arguments));
  }

  void exit_function() {
    assert(!stack().empty());
    stack().pop_back();
  }

  const auto &currentFrame() const {
    assert(!stack().empty());
    return stack().back();
  }
  auto &curentFrame() {
    return const_cast<InterpreterFrame &>(
        const_cast<const InterpreterStack&>(*this).currentFrame());
  }

  // Return null if there is only one frame
  const auto *prevFrame() const {
    assert(!stack().empty());
    return stack().size() <= 1 ? nullptr : &stack()[stack().size() - 2];
  }
  auto prevFrame() {
    return const_cast<InterpreterFrame *>(
        const_cast<const InterpreterStack&>(*this).prevFrame());
  }

  auto numFrames() const { return stack().size(); }

  const InterpreterFrame getFrame(std::size_t Idx) {
    assert(Idx < stack().size());
    return stack()[Idx];
  }

  bool empty() { return stack().empty(); }

private:
  std::vector<InterpreterFrame> &stack() { return Stack; }
  const std::vector<InterpreterFrame> &stack() const { return Stack; }

private:
  std::vector<InterpreterFrame> Stack;
};

class Interpreter final: public Bytecode::InstructionVisitor {
public:
  Interpreter(std::string InitialFuncName,
              std::vector<std::any> Arguments) {
    stack().enter_function(std::move(InitialFuncName), std::move(Arguments));
  }

  std::any getRetVal() { return RetVal; }
  bool emptyStack() { return stack().empty(); }

  void visit(const aload_0 &) override;

  void visit(const invokespecial &) override;

  void visit(const java_return &) override;

  void visit(const iconst_0 &) override;

  void visit(const ireturn &) override;

private:
  InterpreterStack &stack() { return Stack; }
  const InterpreterStack &stack() const { return Stack; }

private:
  InterpreterStack Stack;
  std::any RetVal;
};

void Interpreter::visit(const iconst_0 &) {
  stack().curentFrame().pushSingle<JavaInt>(0);
}

void Interpreter::visit(const ireturn &) {
  // TODO: pop frame and push result to stack
  assert(stack().numFrames() == 1); // function calls are not supported

  RetVal = stack().curentFrame().popSingle<JavaInt>();
  stack().exit_function();
}

void Interpreter::visit(const aload_0 &) {
  assert(false); // Not implemented
}

void Interpreter::visit(const invokespecial &) {
  assert(false); // Not implemented
}

void Interpreter::visit(const java_return &) {
  assert(false); // Not implemented
}

}

std::any SlowInterpreter::Interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<std::any> &InputArguments) {

  Interpreter I(Method.getName(), InputArguments);

  for (const auto &Instr: Method) {
    Instr.accept(I);
  }

  assert(I.emptyStack()); // should exit all functions
  return I.getRetVal();
}
