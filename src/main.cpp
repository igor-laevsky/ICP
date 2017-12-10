#include <iostream>
#include <memory>

#include "JavaTypes/JavaClass.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"
#include "SlowInterpreter/SlowInterpreter.h"
#include "SlowInterpreter/Value.h"

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
      Ret.getAs<SlowInterpreter::JavaInt>();

  return 0;
}
