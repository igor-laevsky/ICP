//
// Verifier implementation
//

#include "Verifier.h"

#include "JavaTypes/StackFrame.h"
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
class MethodVerifier final : public InstructionVisitor {
public:
  explicit MethodVerifier(const JavaMethod &Method) :
      Method(Method),
      CP(Method.getOwner().getConstantPool()) {

    // Set up initial frame
    std::vector<Type> LocalTypes;
    std::tie(ReturnType, LocalTypes) =
        Type::parseMethodDescriptor(Method.getDescriptor());

    // Add 'this' argument type
    if (!(Method.getAccessFlags() & JavaMethod::AccessFlags::ACC_STATIC)) {
      if (Method.getName() == "<init>")
        LocalTypes.insert(LocalTypes.begin(), Types::UninitializedThis);
      else
        LocalTypes.insert(LocalTypes.begin(), Types::Class);
    }

    if (LocalTypes.size() > Method.getMaxLocals())
      throwErr("Too much locals");

    // Materialize stack map
    StackMap = Method.getStackMapBuilder().createTable(LocalTypes);
    StackMapIt = StackMap.begin();

    CurrentFrame = StackFrame(LocalTypes, {});
    CurrentFrame.resizeLocals(Method.getMaxLocals());
  }

  void visit(const aload_0 &) override;
  void visit(const aload &Inst) override;
  void visit(const invokespecial &Inst) override;
  void visit(const java_return &) override;
  void visit(const iconst_val &) override;
  void visit(const ireturn &) override;
  void visit(const dreturn &) override;
  void visit(const putstatic &) override;
  void visit(const getstatic &) override;
  void visit(const dconst_val &) override;
  void visit(const if_icmp_op &) override;
  void visit(const iload_val &) override;
  void visit(const istore_val &) override;
  void visit(const iinc &) override;
  void visit(const java_goto &) override;
  void visit(const iadd &) override;

  // Runs before visiting instruction.
  void runPreConditions(const Instruction &CurInstr) {
    const auto cur_bci = Method.getBciForInst(CurInstr);

    // Must have stack map if after goto
    if (afterGoto) {
      if (StackMapIt == StackMap.end() || StackMapIt.getBci() != cur_bci)
        throwErr("Couldn't find stack map after goto");

      CurrentFrame = *StackMapIt;
      CurrentFrame.resizeLocals(Method.getMaxLocals());

      ++StackMapIt;
      afterGoto = false;
      return;
    }

    // No stack map - nothing to do.
    if (StackMapIt == StackMap.end())
      return;

    // Don't have stack map for the current bci - nothing to do.
    if (StackMapIt.getBci() != cur_bci)
      return;

    const auto &map_frame = *StackMapIt;
    if (!CurrentFrame.transformInto(map_frame))
      throwErr("Current frame is unassignable into map frame");

    CurrentFrame.resizeLocals(Method.getMaxLocals());
    ++StackMapIt;
  }

  // Runs after visit of the instruction.
  void runPostConditions() const {
    if (CurrentFrame.numStack() > Method.getMaxStack())
      throwErr("Exceeded maximum stack size");
  };

  void tryTypeTransition(const std::vector<Type> &ToPop, Type ToPush) {
    if (!CurrentFrame.doTypeTransition(ToPop, ToPush))
      throwErr("Incorrect type transition");
  }

private:
  // Load type 'T' from the loacal variable Idx.
  void loadFromLocal(Bytecode::IdxType Idx, Type ToT) {
    checkLocalIdx(Idx);

    Type FromType = CurrentFrame.getLocal(Idx);
    if (!Types::isAssignable(FromType, ToT))
      throwErr("Local variable has incompatible load type at index " +
               std::to_string(Idx));

    CurrentFrame.pushList({FromType});
  }

  // Store type 'T' into local variable Idx.
  void storeToLocal(Bytecode::IdxType Idx, Type FromT) {
    checkLocalIdx(Idx);

    Type ToT = CurrentFrame.getLocal(Idx);
    if (!Types::isAssignable(FromT, ToT))
      throwErr("Local variable has incompatible store type at index " +
               std::to_string(Idx));

    CurrentFrame.setLocal(Idx, FromT);
  }

  // Checks if we can jump to this target from the current state.
  void targetIsTypeSafe(Bytecode::BciType Bci);

  // Helper to throw a varification error.
  void throwErr(std::string_view Str) const {
    throw VerificationError(Str.data());
  }

  // Pops type list from the current frame or throws verification error.
  void tryPop(const std::vector<Type> &ToPop, std::string_view ErrMsg) {
    if (!CurrentFrame.popMatchingList(ToPop))
      throwErr(ErrMsg);
  }

  // Check if we can access given local and throw if not.
  void checkLocalIdx(Bytecode::IdxType Idx) {
    if (Idx >= CurrentFrame.numLocals())
      throwErr("Incorrect local variable index " + std::to_string(Idx));
  }

private:
  const JavaMethod &Method;
  const ConstantPool &CP;

  StackFrame CurrentFrame{{},
                          {}};
  Type ReturnType = Types::Top;

  StackMapTable StackMap;
  StackMapTable::Iterator StackMapIt;

  // If tru we need to load new stack frame
  bool afterGoto = false;
};

}

void MethodVerifier::visit(const aload_0 &) {
  loadFromLocal(0, Types::Reference);
}

void MethodVerifier::visit(const aload &Inst) {
  loadFromLocal(Inst.getIdx(), Types::Reference);
}

void MethodVerifier::visit(const invokespecial &Inst) {
  const auto *MRef =
      CP.getAsOrNull<ConstantPoolRecords::MethodRef>(Inst.getIdx());
  if (MRef == nullptr)
    throw VerificationError("Incorrect CP index at invokespecial");

  assert(MRef->getName() == "<init>"); // other calls are not yet supported

  std::vector<Type> ArgTypes;
  Type CallRetType = Types::Void;
  std::tie(CallRetType, ArgTypes) =
    Type::parseMethodDescriptor(MRef->getDescriptor());

  if (CallRetType != Types::Void)
    throw VerificationError("<init> method should have void return type");

  // Pop method arguments
  std::reverse(ArgTypes.begin(), ArgTypes.end());
  tryPop(ArgTypes, "Unable to pop arguments");

  // Pop UninitializedArg
  // TODO: Support uninitialized(Address)
  Type UninitializedArg = Types::UninitializedThis;
  Type UninitializedRepl = Types::Class;
  tryPop({UninitializedArg}, "Unable to pop uninitializedThis");

  // Replace UninitializedArg in locals
  for (std::size_t i = 0; i < CurrentFrame.numLocals(); ++i)
    if (CurrentFrame.getLocal(i) == UninitializedArg)
      CurrentFrame.setLocal(i, UninitializedRepl);

  // TODO: Replace UninitializedArg on the stack
}

void MethodVerifier::visit(const java_return &) {
  if (ReturnType != Types::Void)
    throw VerificationError("Return type should be 'void'");
  if (CurrentFrame.flagThisUninit())
    throw VerificationError("Exiting <init> method before complete initialization");
  afterGoto = true;
}

void MethodVerifier::visit(const iconst_val &) {
  CurrentFrame.pushList({Types::Int});
}

void MethodVerifier::visit(const ireturn &) {
  if (ReturnType != Types::Int)
    throw VerificationError("Return type should be integer");

  tryPop({Types::Int}, "Expected integer type to be on the stack");
  afterGoto = true;
}

void MethodVerifier::visit(const dconst_val &) {
  CurrentFrame.pushList({Types::Double});
}

void MethodVerifier::visit(const dreturn &) {
  if (ReturnType != Types::Double)
    throw VerificationError("Return type should be double");

  tryPop({Types::Double}, "Expected double type to be on the stack");
  afterGoto = true;
}

// Helper with common parts of the get and put static bytecodes
Type getFieldType(ConstantPool::IndexType Idx, const ConstantPool &CP) {
  const auto *CPRef = CP.getAsOrNull<ConstantPoolRecords::FieldRef>(Idx);
  if (!CPRef)
    throw VerificationError("Incorrect CP index");

  Type FieldType = Types::Void;
  try {
    FieldType = Type::parseFieldDescriptor(CPRef->getDescriptor());
  } catch (Type::ParsingError &) {
    throw VerificationError("Unable to parse field descriptor");
  }
  assert(FieldType != Types::Void);

  return FieldType;
}

void MethodVerifier::visit(const putstatic &Inst) {
  auto FieldType = getFieldType(Inst.getIdx(), CP);
  tryPop({FieldType}, "Incompatible type in put static instruction");
}

void MethodVerifier::visit(const getstatic &Inst) {
  auto FieldType = getFieldType(Inst.getIdx(), CP);
  CurrentFrame.pushList({FieldType});
}

void MethodVerifier::targetIsTypeSafe(Bytecode::BciType Bci) {
  const auto target_frame = StackMap.findAtBci(Bci);

  if (target_frame == StackMap.end())
    throwErr("Unable to find stack map table entry for the target bci");

  if (!StackFrame::isAssignable(CurrentFrame, *target_frame))
    throwErr("Can't transfer to the target stack frame");
}

void MethodVerifier::visit(const if_icmp_op &Inst) {
  tryPop({Types::Int, Types::Int}, "Incorrect if_icmp operands");
  targetIsTypeSafe(Method.getBciForInst(Inst.getInst()) + Inst.getIdx());
}

void MethodVerifier::visit(const iload_val &Inst) {
  loadFromLocal(Inst.getVal(), Types::Int);
}

void MethodVerifier::visit(const istore_val &Inst) {
  tryPop({Types::Int}, "Expected integer on the stack");
  storeToLocal(Inst.getVal(), Types::Int);
}

void MethodVerifier::visit(const iinc &Inst) {
  checkLocalIdx(Inst.getIdx());

  if (CurrentFrame.getLocal(Inst.getIdx()) != Types::Int)
    throwErr("iinc can only increment integer types");
}

void MethodVerifier::visit(const java_goto &Inst) {
  targetIsTypeSafe(Method.getBciForInst(Inst) + static_cast<int16_t>(Inst.getIdx()));
  afterGoto = true;
  // Reset frame to avoid unfortunate accidents
  CurrentFrame = StackFrame({}, {});
}

void MethodVerifier::visit(const iadd &) {
  tryTypeTransition({Types::Int, Types::Int}, Types::Int);
}



void Verifier::verifyMethod(const JavaMethod &Method) {
  // TODO: Add method level verification

  MethodVerifier V(Method);
  for (const auto &Instr: Method) {
    V.runPreConditions(Instr);

    Instr.accept(V);

    V.runPostConditions();
  }
}

void Verifier::verify(const JavaClass &Class) {
  // TODO: Add class level verification

  for (const auto& Method: Class.methods()) {
    verifyMethod(*Method);
  }
}
