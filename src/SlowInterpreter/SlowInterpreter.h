//
// Very simple interpreter for the java methods
//

#ifndef ICP_INTERPRETER_H
#define ICP_INTERPRETER_H

#include <any>
#include <vector>
#include <cstdint>

#include "JavaTypes/JavaTypesFwd.h"
#include "Runtime/Value.h"
#include "Runtime/ClassManager.h"

namespace SlowInterpreter {

// Expects verified method and returns it's result if it's specified.
Runtime::Value interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<Runtime::Value> &InputArguments,
    Runtime::ClassManager &CM,
    bool Debug = false);

}

#endif //ICP_INTERPRETER_H
