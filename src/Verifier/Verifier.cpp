//
// Verifier implementation
//

#include "Verifier.h"

#include "StackFrame.h"
#include "Bytecode/InstructionVisitor.h"

#include "JavaTypes/JavaClass.h"
#include "Bytecode/Instructions.h"

using namespace JavaTypes;
using namespace Bytecode;
using namespace Verifier;

namespace {

// This visitor is intended to be called on all instructions of the method
// in order of their appearance. Caller is responsible to supply
// correct stack frames when necessary.
class MethodVerifier final: public InstructionVisitor {
public:
  explicit MethodVerifier(const JavaMethod &Method):
      Method(Method),
      CP(Method.getOwner().getConstantPool()) {

    // Set up initial frame
    std::vector<Type> LocalTypes;
    std::tie(ReturnType, LocalTypes) =
      StackFrame::parseMethodDescriptor(Method.getDescriptor());

    // Add 'this' argument type
    if (!(Method.getAccessFlags() & JavaMethod::AccessFlags::ACC_STATIC)) {
      if (Method.getName() == "<init>")
        LocalTypes.insert(LocalTypes.begin(), Types::UninitializedThis);
      else
        LocalTypes.insert(LocalTypes.begin(), Types::Class);
    }

    if (LocalTypes.size() > Method.getMaxLocals())
      throw VerificationError("Exceeded maximum number of locals");

    CurrentFrame = StackFrame(LocalTypes, {});
  }

  void visit(const aload_0 &) override;
  void visit(const aload &Inst) override;
  void visit(const invokespecial &Inst) override;
  void visit(const java_return &) override;
  void visit(const iconst_0 &) override;
  void visit(const ireturn &) override;


  // These two functions are called before or after processing an instruction.
  bool checkPreConditions() const {
    // TODO: This will be used to ensure that we encountered a stack map
    // after branch
    return true;
  }

  bool checkPostConditions() const {
    return CurrentFrame.numStack() <= Method.getMaxStack();
  };

private:
  // Load type 'T' from the loacal variable Idx
  void loadFromLocal(uint32_t Idx, Type T) {
    if (Idx >= CurrentFrame.numLocals())
      throw VerificationError(
          "Unable to load local variable at index " + std::to_string(Idx));

    Type ActualType = CurrentFrame.getLocal(Idx);
    if (!Types::isAssignable(ActualType, T))
      throw VerificationError(
          "Local variable has incompatible type at index " + std::to_string(Idx));

    CurrentFrame.pushList({ActualType});
  }


private:
  const JavaMethod &Method;
  const ConstantPool &CP;

  StackFrame CurrentFrame{{}, {}};
  Type ReturnType = Types::Top;
};

void MethodVerifier::visit(const aload_0 &) override {
  loadFromLocal(0, Types::Reference);
}

void MethodVerifier::visit(const aload &Inst) override {
  loadFromLocal(Inst.getIdx(), Types::Reference);
}

void MethodVerifier::visit(const invokespecial &Inst) override {
  const auto *MRef =
      CP.getAsOrNull<ConstantPoolRecords::MethodRef>(Inst.getIdx());
  if (MRef == nullptr)
    throw VerificationError("Incorrect CP index at invokespecial");

  assert(MRef->getName() == "<init>"); // other calls are not yet supported

  std::vector<Type> ArgTypes;
  Type CallRetType = Types::Void;
  std::tie(CallRetType, ArgTypes) =
    StackFrame::parseMethodDescriptor(MRef->getDescriptor());

  if (CallRetType != Types::Void)
    throw VerificationError("<init> method should have void return type");

  // Pop method arguments
  std::reverse(ArgTypes.begin(), ArgTypes.end());
  if (!CurrentFrame.popMatchingList(ArgTypes))
    throw VerificationError("Unable to pop arguments");

  // Pop UninitializedArg
  // TODO: Support uninitialized(Address)
  Type UninitializedArg = Types::UninitializedThis;
  Type UninitializedRepl = Types::Class;

  if (!CurrentFrame.popMatchingList({UninitializedArg}))
    throw VerificationError("Unable to pop uninitializedThis");

  // Replace UninitializedArg in locals
  for (std::size_t i = 0; i < CurrentFrame.numLocals(); ++i)
    if (CurrentFrame.getLocal(i) == UninitializedArg)
      CurrentFrame.setLocal(i, UninitializedRepl);

  // TODO: Replace UninitializedArg on the stack
}

void MethodVerifier::visit(const java_return &) override {
  if (ReturnType != Types::Void)
    throw VerificationError("Return type should be 'void'");
  if (CurrentFrame.flagThisUninit())
    throw VerificationError("Exiting <init> method before complete initialization");
}

void MethodVerifier::visit(const iconst_0 &) override {
  CurrentFrame.pushList({Types::Int});
}

void MethodVerifier::visit(const ireturn &) override {
  if (ReturnType != Types::Int)
    throw VerificationError("Return type should be integer");

  if (!CurrentFrame.popMatchingList({Types::Int}))
    throw VerificationError("Expected integer type to be on the stack");
}

}

static void verifyMethod(const JavaMethod &Method) {
  // TODO: Add method level verification

  MethodVerifier V(Method);
  for (const auto &Instr: Method) {
    // TODO: Set up StackMap if method has it specified for the current bci

    if (!V.checkPreConditions())
      throw VerificationError(
          "Failed pre conditions at the bci " + std::to_string(Instr.getBci()));

    Instr.accept(V);

    if (!V.checkPostConditions())
      throw VerificationError(
          "Failed post conditions at the bci " + std::to_string(Instr.getBci()));
  }
}

void Verifier::verify(const JavaClass &Class) {
  // TODO: Add class level verification

  for (const auto& Method: Class.getMethods()) {
    verifyMethod(*Method);
  }
}
