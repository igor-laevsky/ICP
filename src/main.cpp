#include <iostream>
#include <memory>

#include "JavaTypes/JavaClass.h"
#include "ClassFileReader/ClassFileReader.h"
#include "Verifier/Verifier.h"

int main() {
  std::unique_ptr<JavaTypes::JavaClass> NewClass;

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

  std::string ErrorMessage;
  //Verifier::verify(*NewClass);

  NewClass->print(std::cout);

  return 0;
}
