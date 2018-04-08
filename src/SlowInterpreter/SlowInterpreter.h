///
/// Very simple interpreter for the java methods.
///

#ifndef ICP_INTERPRETER_H
#define ICP_INTERPRETER_H

#include "JavaTypes/JavaTypesFwd.h"
#include "Runtime/RuntimeFwd.h"

#include <vector>

namespace SlowInterpreter {

// Expects verified method and returns it's result if it's specified.
Runtime::Value interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<Runtime::Value> &InputArguments,
    Runtime::ClassManager &CM,
    bool Debug = false);

}

#endif //ICP_INTERPRETER_H
