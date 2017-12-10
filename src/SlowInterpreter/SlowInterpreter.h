//
// Very simple interpreter for the java methods
//

#ifndef ICP_INTERPRETER_H
#define ICP_INTERPRETER_H

#include <any>
#include <vector>
#include <cstdint>

#include "JavaTypes/JavaTypesFwd.h"
#include "SlowInterpreter/Value.h"

namespace SlowInterpreter {

// Expects verified method and returns it's result if it's specified.
Value interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<Value> &InputArguments,
    bool Debug = false);

}

#endif //ICP_INTERPRETER_H
