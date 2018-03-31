//
// Implementation of the interpreter.
//

#include "SlowInterpreter.h"

#include "Bytecode/InstructionVisitor.h"
#include "Bytecode/Instructions.h"
#include "JavaTypes/JavaMethod.h"
#include "Runtime/Value.h"
#include "Runtime/ClassManager.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"
#include "Bytecode/Instructions.h"

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
using namespace JavaTypes;

namespace {

// Represents single interpreter frame.
// All dynamic cast functions are expected to succeed for the methods which
// passed verification.
class InterpreterFrame final {
public:
  explicit InterpreterFrame(
      const JavaMethod &Method,
      std::vector<Value> Locals) noexcept:
    Method(Method),
    Locals(std::move(Locals)),
    CurInstr(Method.begin()) {

    // Ensure there is always anough locals
    if (Method.getMaxLocals() > Locals.size())
      locals().resize(Method.getMaxLocals());
  }

  bool empty() const { return stack().empty(); }

  const Instruction &getCurInstr() const {
    assert(CurInstr != Method.end());
    return **CurInstr;
  }

  BciType getCurBci() const {
    assert(CurInstr != Method.end());
    return CurInstr.getBci();
  }

  void jumpToBciOffset(BciOffsetType Offset) {
    assert(CurInstr != Method.end());
    CurInstr = Method.getInstrAtOffset(CurInstr, Offset);
    assert(CurInstr != Method.end());
  }

  void jumpToNextInstr() {
    assert(CurInstr != Method.end());
    ++CurInstr;
  }

  Value getLocal(uint32_t Idx) const {
    assert(Idx < locals().size());
    return locals()[Idx];
  }
  template<class T>
  T getLocal(uint32_t Idx) const {
    return getLocal(Idx).getAs<T>();
  }

  void setLocal(uint32_t Idx, Value NewVal) {
    assert(Idx < locals().size());
    locals()[Idx] = NewVal;
  }
  template<class T>
  void setLocal(uint32_t Idx, const std::remove_reference_t<T>& NewVal) {
    setLocal(Idx, Value::create<T>(NewVal));
  }

  Value pop() {
    assert(!stack().empty());

    auto Ret = stack().back();
    stack().pop_back();
    return Ret;
  }

  template<class T>
  T pop() {
    auto Ret = pop();
    return Ret.getAs<T>();
  }

  void push(const Value& Val) {
    stack().push_back(Val);
  }

  template<class T>
  void push(const std::remove_reference_t<T>& Val) {
    stack().push_back(Value::create<T>(Val));
  }


  const JavaMethod &method() const { return Method; }

  void print(std::ostream &Out = std::cout) {
    Out << "Frame for: " << method().getName() << "\n";

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
  const JavaMethod &Method;
  std::vector<Value> Locals;
  std::vector<Value> Stack;

  JavaMethod::CodeIterator CurInstr;
};

// Represents stack of InterpreterFrames.
// All reasonable invariants (like not exiting function before entering any) are
// enforced via asserts since they were already checked by the class verifier.
class InterpreterStack final {
public:
  void enter_function(
      const JavaMethod &Method,
      std::vector<Value> Arguments) {
    stack().emplace_back(Method, std::move(Arguments));
  }

  void exit_function() {
    assert(!stack().empty()); // can't exit if no functions were called
    stack().pop_back();
  }

  const InterpreterFrame &currentFrame() const {
    assert(!stack().empty());
    return stack().back();
  }
  InterpreterFrame &currentFrame() {
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
  Interpreter(
      const JavaMethod &Method,
      std::vector<Value> Arguments,
      ClassManager &CM): CM(CM) {
    stack().enter_function(Method, std::move(Arguments));
  }

  // Main interface method.
  // Executes single instruction and jumps to the next when possible.
  // Return false when there are no instructions left, true otherwise.
  bool runSingleInstr();

  // Accessors for the current interpreter state
  bool empty() { return stack().empty(); }
  const Instruction &getCurInstr() const { return curFrame().getCurInstr(); }
  BciType getCurBci() const { return curFrame().getCurBci(); }
  Value getRetVal() { return RetVal; }

  // Print state of the interpreter. Inteded for the debugging purposes.
  void print(std::ostream &Out = std::cout) { Stack.print(Out); }

  // Meat of the interpreter.
  void visit(const aload_0 &) override;
  void visit(const invokespecial &) override;
  void visit(const iconst_val &) override;
  void visit(const dconst_val &) override;
  void visit(const ireturn &) override;
  void visit(const dreturn &) override;
  void visit(const java_return &) override;
  void visit(const aload &) override;
  void visit(const putstatic &) override;
  void visit(const getstatic &) override;

  void visit(const if_icmp_op &) override;

  void visit(const iload_val &) override;
  void visit(const istore_val &) override;

  void visit(const iinc &) override;

  void visit(const java_goto &) override;
  void visit(const iadd &) override;
  void visit(const java_new &) override;

private:
  InterpreterStack &stack() { return Stack; }
  const InterpreterStack &stack() const { return Stack; }

  InterpreterFrame &curFrame() { return stack().currentFrame(); }
  const InterpreterFrame &curFrame() const { return stack().currentFrame(); }

  const JavaMethod &curMethod() const { return curFrame().method(); }
  const JavaClass &curClass() const { return curFrame().method().getOwner(); }
  const ClassLoader &curLoader() const {
    const auto *loader = CM.getDefLoader(curFrame().method().getOwner());
    assert(loader); // current class should be loaded
    return *loader;
  }

  const ConstantPool &CP() {
    return curMethod().getOwner().getConstantPool();
  }

  void returnFromFunction();

  // Schedules bci jump which is performed in the 'runSingleInstr' method.
  void jumpToBciOffset(BciOffsetType Offset) { NextOffset = Offset; }

private:
  InterpreterStack Stack;
  Value RetVal;

  // Bci offset of the next instruction to execute.
  // Empty means jump to the next instruction.
  // Zero value means do not jump anywhere.
  std::optional<BciOffsetType> NextOffset;

  ClassManager &CM;
};

}

bool Interpreter::runSingleInstr() {
  // Execute current instruction
  getCurInstr().accept(*this);

  // If we exited the last function - we are done.
  if (empty())
    return false;

  // Jump to the next instruction.
  if (NextOffset)
    curFrame().jumpToBciOffset(*NextOffset);
  else
    curFrame().jumpToNextInstr();

  NextOffset = std::nullopt;
  return true;
}

void Interpreter::visit(const iconst_val &Inst) {
  curFrame().push<JavaInt>(Inst.getVal());
}

void Interpreter::visit(const dconst_val &Inst) {
  curFrame().push<JavaDouble>(Inst.getVal());
}

void Interpreter::returnFromFunction() {
  // Pop the result value
  std::optional<Value> result = std::nullopt;
  if (!curFrame().empty())
    result = curFrame().pop();

  // Pop stack frame (which should be empty by now)
  stack().exit_function();

  // If has the result - push it onto the previous stack frame
  if (!result)
    return;

  if (empty()) {
    RetVal = *result;
  } else {
    curFrame().push(*result);
  }
}

void Interpreter::visit(const ireturn &) {
  returnFromFunction();
}

void Interpreter::visit(const dreturn &) {
  returnFromFunction();
}

void Interpreter::visit(const java_return &) {
  returnFromFunction();
}

void Interpreter::visit(const aload_0 &) {
  assert(false); // Not implemented
}

void Interpreter::visit(const invokespecial &Inst) {
  const auto &m_ref = CP().getAs<ConstantPoolRecords::MethodRef>(Inst.getIdx());

  // Resolve the class
  auto &class_obj = CM.getClassObject(m_ref.getClassName(), curLoader());

  // Resolve the method (so far only instance init methods)
  // TODO: This should be a proper resolution with proper exceptions
  assert(m_ref.getName() == "<init>");
  const auto *method = class_obj.getClass().getMethod("<init>");
  assert(method); // should be present

  // Gather arguments
  Type RetTy = Types::Void;
  std::vector<Type> ArgTypes;
  std::tie(RetTy, ArgTypes) = Type::parseMethodDescriptor(method->getDescriptor());
  std::vector<Value> ArgVals;

  ArgVals.reserve(ArgTypes.size());
  for (std::size_t i = 0; i < ArgTypes.size(); ++i) {
    ArgVals.push_back(curFrame().pop());
  }

  // Start new function
  stack().enter_function(*method, std::move(ArgVals));
  NextOffset = 0;
}

void Interpreter::visit(const aload &) {
  assert(false); // Not implemented
}

void Interpreter::visit(const putstatic &Inst) {
  const auto &FRef = CP().getAs<ConstantPoolRecords::FieldRef>(Inst.getIdx());
  auto &class_obj = CM.getClassObject(FRef.getClassName(), curLoader());

  class_obj.setField(FRef.getName(), curFrame().pop());
}

void Interpreter::visit(const getstatic &Inst) {
  const auto &FRef = CP().getAs<ConstantPoolRecords::FieldRef>(Inst.getIdx());
  auto &class_obj = CM.getClassObject(FRef.getClassName(), curLoader());

  curFrame().push(class_obj.getField(FRef.getName()));
}

// Helper for the comparison operators. Receives two stack values and calls
// relevant c++ operator on them.
template<typename ValT>
static bool javaCompare(
    ComparisonOp CmpOp, const Value &Val1, const Value &Val2) {

  const auto real_val1 = Val1.getAs<ValT>();
  const auto real_val2 = Val2.getAs<ValT>();

  if (CmpOp == COMP_EQ)
    return real_val1 == real_val2;
  if (CmpOp == COMP_NE)
    return real_val1 != real_val2;
  if (CmpOp == COMP_LT)
    return real_val1 < real_val2;
  if (CmpOp == COMP_GE)
    return real_val1 >= real_val2;
  if (CmpOp == COMP_GT)
    return real_val1 > real_val2;
  if (CmpOp == COMP_LE)
    return real_val1 <= real_val2;

  assert(false); // unrecognised comparison operator
  return false;
}

void Interpreter::visit(const if_icmp_op &Inst) {
  // Note the ordering here according to the jvm specification
  const auto val2 = curFrame().pop();
  const auto val1 = curFrame().pop();

  const auto cmp_op = static_cast<ComparisonOp>(Inst.getVal());
  const bool res = javaCompare<JavaInt>(cmp_op, val1, val2);

  if (res) {
    jumpToBciOffset(Inst.getIdx());
  }
}

void Interpreter::visit(const istore_val &Inst) {
  curFrame().setLocal(Inst.getVal(), curFrame().pop());
}

void Interpreter::visit(const iload_val &Inst) {
  curFrame().push(curFrame().getLocal(Inst.getVal()));
}

void Interpreter::visit(const iinc &Inst) {
  const auto cur_val = curFrame().getLocal<JavaInt>(Inst.getIdx());
  curFrame().setLocal<JavaInt>(Inst.getIdx(), cur_val + Inst.getConst());
}

void Interpreter::visit(const java_goto &Inst) {
  jumpToBciOffset(Inst.getIdx());
}

void Interpreter::visit(const iadd &) {
  const auto val1 = curFrame().pop<JavaInt>();
  const auto val2 = curFrame().pop<JavaInt>();

  // TODO: Should properly handle overflow
  curFrame().push<JavaInt>(val1 + val2);
}

void Interpreter::visit(const java_new &Inst) {
  const auto &class_ref = CP().getAs<ConstantPoolRecords::ClassInfo>(Inst.getIdx());

  // Resolve the class (also load, verify and initialize it if necessary)
  auto &class_obj = CM.getClassObject(class_ref.getName(), curLoader());

  // Create new instance of this class and push it on the stack
  JavaRef instance = InstanceObject::create(class_obj);
  curFrame().push<JavaRef>(instance);
}



Value SlowInterpreter::interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<Value> &InputArguments,
    Runtime::ClassManager &CM,
    bool Debug /*= false*/) {

  Interpreter I(Method, InputArguments, CM);

  // TODO: assert that all required arguments are specified

  do {
    if (Debug) {
      std::cout << "#" << I.getCurBci() << " ";
      I.getCurInstr().print(std::cout);
      I.print();
    }
  } while (I.runSingleInstr());

  assert(I.empty()); // should exit all functions
  return I.getRetVal();
}
