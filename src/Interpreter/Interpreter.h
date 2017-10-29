//
// Very simple interpreter for the java methods
//

#ifndef ICP_INTERPRETER_H
#define ICP_INTERPRETER_H

#include <any>
#include <vector>
#include <cstdint>

#include "JavaTypes/JavaTypesFwd.h"

namespace SlowInterpreter {

using JavaInt = uint32_t;

// Expects verified method and returns it's result if it's specified.
std::any Interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<std::any> &InputArguments);

}

#endif //ICP_INTERPRETER_H
