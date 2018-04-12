#include "JavaTypes/JavaClass.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"
#include "SlowInterpreter/SlowInterpreter.h"
#include "Runtime/ClassManager.h"

#include <iostream>
#include <memory>

int main() {

  Runtime::ClassManager CM;

  // Load, verify and initialize the class. Throws in case of an error.
  const auto &class_obj =
      CM.getClassObject("./examples/SimpleNew", Runtime::getBootstrapLoader());

  // Interpret
  auto *java_main = class_obj.getMethod("main");
  if (!java_main) {
    std::cerr << "Unable to find main entry point";
    return 1;
  }

  auto Ret = SlowInterpreter::interpret(*java_main, {}, CM, true);

  class_obj.getClass().print(std::cout);

  std::cout << "Interpreter returned: " << Ret.getAs<Runtime::JavaInt>();

  return 0;
}
