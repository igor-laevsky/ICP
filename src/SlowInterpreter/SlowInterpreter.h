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

// Expects verified method and returns it's result if it's specified.
std::any interpret(
    const JavaTypes::JavaMethod &Method,
    const std::vector<std::any> &InputArguments,
    bool Debug = false);

}

#endif //ICP_INTERPRETER_H
