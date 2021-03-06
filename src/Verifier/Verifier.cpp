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
      throwErr("Too many locals");

    // Materialize stack map
    StackMap = Method.getStackMapBuilder().createTable(LocalTypes);
    StackMapIt = StackMap.begin();

    CurrentFrame = StackFrame(LocalTypes, {});
    CurrentFrame.resizeLocals(Method.getMaxLocals());

    CurInstr = Method.begin();
  }

  bool runSungleInstr() {
    if (CurInstr == Method.end())
      return false;

    runPreConditions();
    getCurInstr().accept(*this);
    runPostConditions();

    ++CurInstr;
    return true;
  }

  void visit(const aload_val &Inst) override;
  void visit(const astore_val &Inst) override;
  void visit(const invokespecial &Inst) override;
  void visit(const java_return &) override;
  void visit(const iconst_val &) override;
  void visit(const ireturn &) override;
  void visit(const dreturn &) override;
  void visit(const putstatic &) override;
  void visit(const getstatic &) override;
  void visit(const putfield &) override;
  void visit(const getfield &) override;
  void visit(const dconst_val &) override;
  void visit(const if_icmp_op &) override;
  void visit(const iload_val &) override;
  void visit(const istore_val &) override;
  void visit(const iinc &) override;
  void visit(const java_goto &) override;
  void visit(const iadd &) override;
  void visit(const java_new &) override;
  void visit(const dup &Inst) override;
  void visit(const bipush &) override;

  // Runs before visiting instruction.
  void runPreConditions() {
    // Must have stack map if after goto
    if (afterGoto) {
      if (StackMapIt == StackMap.end() || StackMapIt.getBci() != getCurBci())
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
    if (StackMapIt.getBci() != getCurBci())
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

private:
  // Load type 'T' from the loacal variable Idx.
  void loadFromLocal(Bytecode::IdxType Idx, Type T) {
    checkLocalIdx(Idx);

    Type ActualType = CurrentFrame.getLocal(Idx);
    if (!Types::isAssignable(ActualType, T))
      throwErr("Local variable has incompatible load type at index " +
               std::to_string(Idx));

    CurrentFrame.pushList({ActualType});
  }

  // Store type 'T' into local variable Idx.
  void storeToLocal(Bytecode::IdxType Idx, Type T) {
    checkLocalIdx(Idx);

    std::optional<Type> actual_type = CurrentFrame.popMatchingType(T);
    if (!actual_type)
      throwErr("Unable to pop type for local " + std::to_string(Idx));

    CurrentFrame.setLocal(Idx, *actual_type);
  }

  // Checks if we can jump to this target from the current state.
  void targetIsTypeSafe(Bytecode::BciOffsetType Bci);

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

  const Instruction &getCurInstr() const {
    assert(CurInstr != Method.end());
    return **CurInstr;
  }

  BciType getCurBci() const {
    assert(CurInstr != Method.end());
    return CurInstr.getBci();
  }

  void tryTypeTransition(const std::vector<Type> &ToPop, Type ToPush) {
    if (!CurrentFrame.doTypeTransition(ToPop, ToPush))
      throwErr("Incorrect type transition");
  }

private:
  const JavaMethod &Method;
  const ConstantPool &CP;

  JavaMethod::CodeIterator CurInstr;

  StackFrame CurrentFrame{{}, {}};
  Type ReturnType = Types::Top;

  StackMapTable StackMap;
  StackMapTable::const_iterator StackMapIt;

  // If tru we need to load new stack frame
  bool afterGoto = false;
};

}

void MethodVerifier::visit(const aload_val &Inst) {
  loadFromLocal(Inst.getVal(), Types::Reference);
}

void MethodVerifier::visit(const astore_val &Inst) {
  storeToLocal(Inst.getVal(), Types::Reference);
}

void MethodVerifier::visit(const invokespecial &Inst) {
  const auto *MRef =
      CP.getAsOrNull<ConstantPoolRecords::MethodRef>(Inst.getIdx());
  if (MRef == nullptr)
    throwErr("Incorrect CP index at invokespecial");

  if (MRef->getName() != "<init>") {
    throwErr("Private or super methods is not yet supported");
  }

  std::vector<Type> ArgTypes;
  Type CallRetType = Types::Void;
  std::tie(CallRetType, ArgTypes) =
    Type::parseMethodDescriptor(MRef->getDescriptor());

  if (CallRetType != Types::Void)
    throwErr("<init> method should have void return type");

  // Pop method arguments
  std::reverse(ArgTypes.begin(), ArgTypes.end());
  tryPop(ArgTypes, "Unable to pop arguments");

  // Pop UninitializedArg
  if (CurrentFrame.emptyStack()) {
    throwErr("Unable to pop uninitialized arg: stack is empty");
  }
  Type UninitializedArg = CurrentFrame.topStack();
  if (UninitializedArg != Types::UninitializedOffset() &&
      UninitializedArg != Types::UninitializedThis) {
    throwErr("Expected uninitialized arg on the stack");
  }

  // TODO: This should choose class which was symbolically referenced by the
  // invokespecial instruction.
  Type UninitializedRepl = Types::Class;
  tryPop({UninitializedArg}, "Unable to pop uninitializedThis");

  // Replace UninitializedArg in locals and on the stack
  CurrentFrame.substituteLocals(UninitializedArg, UninitializedRepl);
  CurrentFrame.substituteStack(UninitializedArg, UninitializedRepl);
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
static Type getFieldType(ConstantPool::IndexType Idx, const ConstantPool &CP) {
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

void MethodVerifier::visit(const putfield &Inst) {
  auto field_type = getFieldType(Inst.getIdx(), CP);

  tryPop({field_type}, "Incompatible type in put field instruction");

  // TODO: Protected checks

  if (CurrentFrame.emptyStack())
    throwErr("Unable to pop field class: empty stack");

  if (CurrentFrame.topStack() == Types::UninitializedThis) {
    bool ok = CurrentFrame.popMatchingList({Types::UninitializedThis});
    (void)ok; assert(ok); // just checked
  } else {
    tryPop({Types::Class}, "Unable to pop field holder");
  }
}

void MethodVerifier::visit(const getfield &Inst) {
  auto field_type = getFieldType(Inst.getIdx(), CP);
  tryTypeTransition({Types::Class}, field_type);
  // TODO: Protected checks
}

void MethodVerifier::targetIsTypeSafe(Bytecode::BciOffsetType Off) {
  auto target_bci = getCurBci() + Off;
  auto target_frame = StackMap.findAtBci(target_bci);

  if (target_frame == StackMap.end())
    throwErr("Unable to find stack map table entry for the target bci");

  if (!StackFrame::isAssignable(CurrentFrame, *target_frame))
    throwErr("Can't transfer to the target stack frame");
}

void MethodVerifier::visit(const if_icmp_op &Inst) {
  tryPop({Types::Int, Types::Int}, "Incorrect if_icmp operands");
  targetIsTypeSafe(Inst.getIdx());
}

void MethodVerifier::visit(const iload_val &Inst) {
  loadFromLocal(Inst.getVal(), Types::Int);
}

void MethodVerifier::visit(const istore_val &Inst) {
  storeToLocal(Inst.getVal(), Types::Int);
}

void MethodVerifier::visit(const iinc &Inst) {
  checkLocalIdx(Inst.getIdx());

  if (CurrentFrame.getLocal(Inst.getIdx()) != Types::Int)
    throwErr("iinc can only increment integer types");
}

void MethodVerifier::visit(const java_goto &Inst) {
  targetIsTypeSafe(Inst.getIdx());
  afterGoto = true;
  // Reset frame to avoid unfortunate accidents
  CurrentFrame = StackFrame({}, {});
}

void MethodVerifier::visit(const iadd &) {
  tryTypeTransition({Types::Int, Types::Int}, Types::Int);
}

void MethodVerifier::visit(const java_new &Inst) {
  // Check constant pool index
  const auto *cp_ref = CP.getAsOrNull<ConstantPoolRecords::ClassInfo>(Inst.getIdx());
  if (!cp_ref) {
    throwErr("Constant pool index should point to the ClassInfo " +
             std::to_string(Inst.getIdx()));
  }

  Type new_item = Types::UninitializedOffset(getCurBci());

  // Check that we didn't start initialization at the same offset already
  if (CurrentFrame.stackContains(new_item)) {
    throwErr("Already initializing new item at offset " +
             std::to_string(getCurBci()));
  }

  // Replace new_item with Top in the Locals array
  CurrentFrame.substituteLocals(new_item, Types::Top);

  // Push uninitialized(Offset)
  CurrentFrame.pushList({new_item});
}

void MethodVerifier::visit(const dup &) {
  if (CurrentFrame.emptyStack())
    throwErr("Can't dup from an empty stack");

  const auto actual_type = CurrentFrame.topStack();
  if (Types::sizeOf(actual_type) != 1)
    throwErr("Can only dup values from category 1");

  CurrentFrame.pushList({actual_type});
}

void MethodVerifier::visit(const bipush &) {
  CurrentFrame.pushList({Types::Int});
}




void Verifier::verifyMethod(const JavaMethod &Method) {
  // TODO: Add method level verification

  MethodVerifier V(Method);
  while (V.runSungleInstr()) {
    ; // May add debug output here
  }
}

void Verifier::verify(const JavaClass &Class) {
  // TODO: Add class level verification

  for (const auto &Method: Class.methods()) {
    verifyMethod(*Method);
  }
}
