#include "JavaTypes/JavaClass.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"
#include "SlowInterpreter/SlowInterpreter.h"
#include "Runtime/Value.h"

#include <iostream>
#include <memory>

int main() {
  std::unique_ptr<JavaTypes::JavaClass> NewClass;

  // Load class
  try {
    NewClass = ClassFileReader::loadClassFromFile("./assets/Simple.class");
  } catch (ClassFileReader::FileNotFound &) {
    std::cout << "Couldn't open class file." << "\n";
    return 1;
  } catch (ClassFileReader::FormatError  &e) {
    std::cout << "Wrong class file format: "  << e.what() << "\n";
    return 1;
  }
  assert(NewClass != nullptr);

  // Verify class
  Verifier::verify(*NewClass);

  NewClass->print(std::cout);

  // Interpret
  auto Method = NewClass->getMethod("main");
  assert(Method != nullptr);
  
  auto Ret = SlowInterpreter::interpret(*Method, {}, true);

  std::cout << "Interpreter returned: " <<
      Ret.getAs<Runtime::JavaInt>();

  return 0;
}
