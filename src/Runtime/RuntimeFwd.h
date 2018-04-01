///
/// Runtime forward declarations
///

#ifndef ICP_RUNTIMEFWD_H
#define ICP_RUNTIMEFWD_H

namespace Runtime {

class UnrecognizedField;

class Value;

/// Forward declarations of the entities managed by the GC
///

class Object;
class ClassObject;
class InstanceObject;
class ArrayObject;

/// Runtime data types
///
// No direct support of booleans, but we may choose to optimize them later
using JavaBool = int8_t;
using JavaByte = int8_t;
using JavaChar = uint16_t;
using JavaShort = int16_t;
using JavaInt = int32_t;
using JavaLong = int64_t;
using JavaFloat = float;
using JavaDouble = double;
// Someday this will become GC managed pointer.
using JavaRef = Object*;

// Determine stack type from the given type
template<class T> struct promote_to_stack {};
template<> struct promote_to_stack<JavaByte>   { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaChar>   { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaShort>  { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaInt>    { using Result = JavaInt;    };
template<> struct promote_to_stack<JavaLong>   { using Result = JavaLong;   };
template<> struct promote_to_stack<JavaFloat>  { using Result = JavaFloat;  };
template<> struct promote_to_stack<JavaDouble> { using Result = JavaDouble; };
template<> struct promote_to_stack<JavaRef>    { using Result = JavaRef;    };

template<class T>
using promote_to_stack_t = typename promote_to_stack<T>::Result;

}

#endif //ICP_RUNTIMEFWD_H
