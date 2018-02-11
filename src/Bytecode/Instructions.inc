// Enumerate all instructions

// Enumerate all wrappers

// Instructions with direct visits
// Instructions with forwarding visits

#ifndef HANDLE_INSTR_ALL
  #define HANDLE_INSTR_ALL(ClassName)
#endif

#ifndef HANDLE_INSTR
  #define HANDLE_INSTR(ClassName) HANDLE_INSTR_ALL(ClassName)
#endif

#ifndef HANDLE_INSTR_WRAPPED
  #define HANDLE_INSTR_WRAPPED(ClassName) HANDLE_INSTR_ALL(ClassName)
#endif

#ifndef HANDLE_WRAPPER
  #define HANDLE_WRAPPER(ClassName)
#endif

HANDLE_INSTR(invokespecial)

HANDLE_INSTR_WRAPPED(iconst_m1)
HANDLE_INSTR_WRAPPED(iconst_0)
HANDLE_INSTR_WRAPPED(iconst_1)
HANDLE_INSTR_WRAPPED(iconst_2)
HANDLE_INSTR_WRAPPED(iconst_3)
HANDLE_INSTR_WRAPPED(iconst_4)
HANDLE_INSTR_WRAPPED(iconst_5)

HANDLE_INSTR_WRAPPED(dconst_0)
HANDLE_INSTR_WRAPPED(dconst_1)

HANDLE_INSTR(ireturn)
HANDLE_INSTR(dreturn)
HANDLE_INSTR(java_return)

HANDLE_INSTR(putstatic)
HANDLE_INSTR(getstatic)

HANDLE_INSTR_WRAPPED(if_icmpeq)
HANDLE_INSTR_WRAPPED(if_icmpne)
HANDLE_INSTR_WRAPPED(if_icmplt)
HANDLE_INSTR_WRAPPED(if_icmpge)
HANDLE_INSTR_WRAPPED(if_icmpgt)
HANDLE_INSTR_WRAPPED(if_icmple)

HANDLE_INSTR(java_goto)

HANDLE_INSTR(aload)
HANDLE_INSTR(aload_0)

HANDLE_INSTR_WRAPPED(iload)
HANDLE_INSTR_WRAPPED(iload_0)
HANDLE_INSTR_WRAPPED(iload_1)
HANDLE_INSTR_WRAPPED(iload_2)
HANDLE_INSTR_WRAPPED(iload_3)
HANDLE_INSTR_WRAPPED(istore)
HANDLE_INSTR_WRAPPED(istore_0)
HANDLE_INSTR_WRAPPED(istore_1)
HANDLE_INSTR_WRAPPED(istore_2)
HANDLE_INSTR_WRAPPED(istore_3)

HANDLE_INSTR(iinc)
             
HANDLE_WRAPPER(if_icmp_op)
HANDLE_WRAPPER(iconst_val)
HANDLE_WRAPPER(dconst_val)
HANDLE_WRAPPER(iload_val)
HANDLE_WRAPPER(istore_val)

#undef HANDLE_INSTR_ALL
#undef HANDLE_INSTR
#undef HANDLE_INSTR_WRAPPED
#undef HANDLE_WRAPPER
