//
// Implementation of the interpreter.
//

#include "SlowInterpreter.h"

#include "Bytecode/InstructionVisitor.h"
#include "JavaTypes/JavaMethod.h"
#include "Runtime/Value.h"

#include <cassert>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <ostream>
#include <iostream>

using namespace Bytecode;
using namespace SlowInterpreter;
using namespace Runtime;

namespace {

// Represents single interpreter frame.
// All dynamic cast functions are expected to succeed for the methods which
// passed verification.
class InterpreterFrame final {
public:
  explicit InterpreterFrame(
      std::string FunctionName,
      std::vector<Value> Locals) noexcept:
    FunctionName(std::move(FunctionName)),
    Locals(std::move(Locals)) {
    ;
  }

  template<class T>
  T getLocal(uint32_t Idx) const {
    assert(Idx < locals().size());
    return locals()[Idx].getAs<T>();
  }

  template<class T>
  void setLocal(uint32_t Idx, const std::remove_reference_t<T>& NewVal) {
    assert(Idx < locals().size());
    locals()[Idx] = Value::create<T>(NewVal);
  }

  Value popValue() {
    assert(!stack().empty());

    auto Ret = stack().back();
    stack().pop_back();
    return Ret;
  }

  template<class T>
  T pop() {
    auto Ret = popValue();
    return Ret.getAs<T>();
  }

  template<class T>
  void push(const std::remove_reference_t<T>& Val) {
    stack().push_back(Value::create<T>(Val));
  }

  void print(std::ostream &Out = std::cout) {
    Out << "Frame for: " << FunctionName << "\n";

    Out << "  Locals:\n";
    int Idx = 0;
    for (const auto &L: Locals) {
      Out << "    [" << Idx << "] " << L << "\n";
      Idx++;
    }

    Out << "  Stack:\n";
    Idx = 0;
    for (auto It = Stack.rbegin(); It != Stack.rend(); ++It) {
      Out << "    [" << Idx << "] " << *It << "\n";
      Idx++;
    }
  }

private:
  std::vector<Value> &locals() { return Locals; }
  const std::vector<Value> &locals() const { return Locals; }

  std::vector<Value> &stack() { return Stack; }
  const std::vector<Value> &stack() const { return Stack; }

private:
  std::string FunctionName;
  std::vector<Value> Locals;
  std::vector<Value> Stack;
};

// Represents stack of InterpreterFrames.
// All reasonable invariants (like not exiting function before entering any) are
// enforced via asserts since they were already checked by the class verifier.
class InterpreterStack final {
public:
  void enter_function(
      std::string FunctionName,
      std::vector<Value> Arguments) {
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

  void print(std::ostream &Out = std::cout) {
    int Idx = 0;
    for (auto It = Stack.rbegin(); It != Stack.rend(); ++It) {
      Out << "<<< " << Idx << "\n";
      It->print(Out);
      ++Idx;
    }
  }

private:
  std::vector<InterpreterFrame> &stack() { return Stack; }
  const std::vector<InterpreterFrame> &stack() const { return Stack; }

private:
  std::vector<InterpreterFrame> Stack;
};

class Interpreter final: public Bytecode::InstructionVisitor {
public:
  Interpreter(std::string InitialFuncName,
              std::vector<Value> Arguments) {
    stack().enter_function(std::move(InitialFuncName), std::move(Arguments));
  }

  Value getRetVal() { return RetVal; }
  bool emptyStack() { return stack().empty(); }

  void print(std::ostream &Out = std::cout) { Stack.print(Out); }

  void visit(const aload_0 &) override;
  void visit(const invokespecial &) override;
  void visit(const java_return &) override;
  void visit(const iconst_0 &) override;
  void visit(const ireturn &) override;
  void visit(const aload &) override;
  void visit(const putstatic &) override;
  void visit(const getstatic &) override;

private:
  InterpreterStack &stack() { return Stack; }
  const InterpreterStack &stack() const { return Stack; }

private:
  InterpreterStack Stack;
  Value RetVal;
};

void Interpreter::visit(const iconst_0 &) {
  stack().curentFrame().push<JavaInt>(0);
}

void Interpreter::visit(const ireturn &) {
  // TODO: pop frame and push result to stack
  assert(stack().numFrames() == 1); // function calls are not supported

  RetVal = stack().curentFrame().popValue();
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

void Interpreter::visit(const aload &) {
  assert(false); // Not implemented
}

void Interpreter::visit(const putstatic &) {
  assert(false); // Not implemented
}

void Interpreter::visit(const getstatic &) {
  assert(false); // Not implemented
}

}

Value SlowInterpreter::interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<Value> &InputArguments,
    bool Debug /*= false*/) {

  Interpreter I(Method.getName(), InputArguments);

  // TODO: assert that all required arguments are specified

  for (const auto &Instr: Method) {
    Instr.accept(I);

    if (Debug) {
      std::cout << "#" << Instr.getBci() << " ";
      Instr.print(std::cout);
      I.print();
    }
  }

  assert(I.emptyStack()); // should exit all functions
  return I.getRetVal();
}
