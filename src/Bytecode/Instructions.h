//
// Definitions for the bytecode instructions
//

#ifndef ICP_INSTRUCTIONS_H
#define ICP_INSTRUCTIONS_H

#include "InstructionUtils.h"

namespace Bytecode {

class invokespecial final: public SingleIndex<invokespecial> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb7;
  static constexpr const char *Name = "invokespecial";
};

class java_return final: public NoIndex<java_return> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xb1;
  static constexpr const char *Name = "return";
};

#define DEF_ICONST(Num, Value, OpCodeVal) \
class iconst_##Num final: public NoIndex<iconst_##Num> { \
  using NoIndex::NoIndex; \
\
public:\
  static constexpr uint8_t OpCode = OpCodeVal;\
  static constexpr const char *Name = "iconst_"#Num;\
  static constexpr int8_t Val = Value;\
}

DEF_ICONST(m1, -1, 0x02);
DEF_ICONST(0, 0, 0x03);
DEF_ICONST(1, 1, 0x04);
DEF_ICONST(2, 2, 0x05);
DEF_ICONST(3, 3, 0x06);
DEF_ICONST(4, 4, 0x07);
DEF_ICONST(5, 5, 0x08);

#undef DEF_ICONST

class iconst_val final:
    public ValueInstWrapper<
        iconst_m1, iconst_0, iconst_1,
        iconst_2, iconst_3, iconst_4, iconst_5> {
  using ValueInstWrapper::ValueInstWrapper;
};

class dconst_0 final: public NoIndex<dconst_0> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x0e;
  static constexpr const char *Name = "dconst_0";
  static constexpr const uint8_t Val = 0;
};

class dconst_1 final: public NoIndex<dconst_1> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x0f;
  static constexpr const char *Name = "dconst_1";
  static constexpr const uint8_t Val = 1;
};

class dconst_val final: public ValueInstWrapper<dconst_0, dconst_1> {
  using ValueInstWrapper::ValueInstWrapper;
};

class ireturn final: public NoIndex<ireturn> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xac;
  static constexpr const char *Name = "ireturn";
};

class dreturn final: public NoIndex<dreturn> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0xaf;
  static constexpr const char *Name = "dreturn";
};

class getstatic final: public SingleIndex<getstatic> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb2;
  static constexpr const char *Name = "getstatic";
};

class putstatic final: public SingleIndex<putstatic> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb3;
  static constexpr const char *Name = "putstatic";
};

///
/// Comparisons
///
enum ComparisonOp: uint8_t {
  COMP_EQ = 0,
  COMP_NE,
  COMP_LT,
  COMP_GE,
  COMP_GT,
  COMP_LE
};

#define IF_ICMP(Suffix, OpCodeNum, OpCodeName) \
class if_icmp##Suffix: public OffsetIndex<if_icmp##Suffix> { \
  using SingleIndex::SingleIndex; \
\
public:\
  static constexpr uint8_t OpCode = OpCodeNum;\
  static constexpr const char *Name = "if_icmp"#Suffix;\
  static constexpr uint8_t Val = OpCodeName;\
}

IF_ICMP(eq, 0x9f, COMP_EQ);
IF_ICMP(ne, 0xa0, COMP_NE);
IF_ICMP(lt, 0xa1, COMP_LT);
IF_ICMP(ge, 0xa2, COMP_GE);
IF_ICMP(gt, 0xa3, COMP_GT);
IF_ICMP(le, 0xa4, COMP_LE);

#undef IF_ICMP

class if_icmp_op:
    public ValueIdxWrapper<
      if_icmpeq, if_icmpne, if_icmplt,
      if_icmpge, if_icmpgt, if_icmple> {
  using ValueIdxWrapper::ValueIdxWrapper;
};

class java_goto final: public OffsetIndex<java_goto> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xa7;
  static constexpr const char *Name = "goto";
};

///
/// Locals load/store
///

#define DEF_ILOAD(Value, OpCodeVal) \
class iload_##Value final: public NoIndex<iload_##Value> { \
  using NoIndex::NoIndex; \
\
public:\
  static constexpr uint8_t OpCode = OpCodeVal;\
  static constexpr const char *Name = "iload_"#Value;\
  static constexpr ByteIdxType Val = Value;\
}

#define DEF_ISTORE(Value, OpCodeVal) \
class istore_##Value final: public NoIndex<istore_##Value> { \
  using NoIndex::NoIndex; \
\
public:\
  static constexpr uint8_t OpCode = OpCodeVal;\
  static constexpr const char *Name = "istore_"#Value;\
  static constexpr ByteIdxType Val = Value;\
}

DEF_ILOAD(0, 0x1a); DEF_ISTORE(0, 0x3b);
DEF_ILOAD(1, 0x1b); DEF_ISTORE(1, 0x3c);
DEF_ILOAD(2, 0x1c); DEF_ISTORE(2, 0x3d);
DEF_ILOAD(3, 0x1d); DEF_ISTORE(3, 0x3e);

#undef DEF_ILOAD
#undef DEF_ISTORE

class iload: public ByteIndex<iload> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0x15;
  static constexpr const char *Name = "iload";
};

class istore: public ByteIndex<istore> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0x36;
  static constexpr const char *Name = "istore";
};

class iload_val final:
    public ValueInstWrapper<iload_0, iload_1, iload_2, iload_3> {
public:
  using ValueInstWrapper::ValueInstWrapper;
  iload_val(const iload &Inst): ValueInstWrapper(from_idx, Inst) {}
};

class istore_val final:
    public ValueInstWrapper<istore_0, istore_1, istore_2, istore_3> {
public:
  using ValueInstWrapper::ValueInstWrapper;
  istore_val(const istore &Inst): ValueInstWrapper(from_idx, Inst) {}
};

class iinc final: public VisitableInstruction<iinc> {
public:
  static constexpr uint8_t Length = 3;
  static constexpr uint8_t OpCode = 0x84;
  static constexpr const char *Name = "iinc";

public:
  uint8_t getIdx() const { return Idx; }
  int8_t getConst() const { return Const; }

  void print(std::ostream &Out) const override {
    Out << iinc::Name << " #" << std::to_string(getIdx()) <<  " #" <<
        std::to_string(getConst()) << "\n";
  }

private:
  explicit iinc(ContainerIterator It):
      Idx(*(It + 1)),
      Const(*(It + 2)) {
    ;
  }

  // Allow calling constructor from the Instruction::create functions
  friend class Instruction;

private:
  const uint8_t Idx;
  const int8_t Const;
};

class iadd final: public NoIndex<iadd> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x60;
  static constexpr const char *Name = "iadd";
};

class java_new final: public SingleIndex<java_new> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xbb;
  static constexpr const char *Name = "new";
};

class getfield final: public SingleIndex<getfield> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb4;
  static constexpr const char *Name = "getfield";
};

class putfield final: public SingleIndex<putfield> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0xb5;
  static constexpr const char *Name = "putfield";
};

///
/// Aload astore
///
/// TODO: aload has single-byte index

#define DEF_aload(Value, OpCodeVal) \
class aload_##Value final: public NoIndex<aload_##Value> { \
  using NoIndex::NoIndex; \
\
public:\
  static constexpr uint8_t OpCode = OpCodeVal;\
  static constexpr const char *Name = "aload_"#Value;\
  static constexpr ByteIdxType Val = Value;\
}

#define DEF_astore(Value, OpCodeVal) \
class astore_##Value final: public NoIndex<astore_##Value> { \
  using NoIndex::NoIndex; \
\
public:\
  static constexpr uint8_t OpCode = OpCodeVal;\
  static constexpr const char *Name = "astore_"#Value;\
  static constexpr ByteIdxType Val = Value;\
}

DEF_aload(0, 0x2a); DEF_astore(0, 0x4b);
DEF_aload(1, 0x2b); DEF_astore(1, 0x4c);
DEF_aload(2, 0x2c); DEF_astore(2, 0x4d);
DEF_aload(3, 0x2d); DEF_astore(3, 0x4e);

#undef DEF_aload
#undef DEF_astore

class aload: public ByteIndex<aload> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0x19;
  static constexpr const char *Name = "aload";
};

class astore: public ByteIndex<astore> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0x3a;
  static constexpr const char *Name = "astore";
};

class aload_val final:
    public ValueInstWrapper<aload_0, aload_1, aload_2, aload_3> {
public:
  using ValueInstWrapper::ValueInstWrapper;
  aload_val(const aload &Inst): ValueInstWrapper(from_idx, Inst) {}
};

class astore_val final:
    public ValueInstWrapper<astore_0, astore_1, astore_2, astore_3> {
public:
  using ValueInstWrapper::ValueInstWrapper;
  astore_val(const astore &Inst): ValueInstWrapper(from_idx, Inst) {}
};

class dup: public NoIndex<dup> {
  using NoIndex::NoIndex;

public:
  static constexpr uint8_t OpCode = 0x59;
  static constexpr const char *Name = "dup";
};

class bipush: public ByteIndex<bipush> {
  using SingleIndex::SingleIndex;

public:
  static constexpr uint8_t OpCode = 0x10;
  static constexpr const char *Name = "bipush";
};


}

#endif //ICP_INSTRUCTIONS_H
