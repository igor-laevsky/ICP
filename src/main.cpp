#include <iostream>
#include <memory>

#include "JavaTypes/JavaClass.h"
#include "ClassFileReader/ClassFileReader.h"

int main() {
  std::unique_ptr<JavaTypes::JavaClass> NewClass;

  try {
    NewClass = ClassFileReader::loadClassFromFile("./assets/Simple.class");
  } catch (ClassFileReader::FileNotFound &) {
    std::cout << "Couldn't open class file." << "\n";
  } catch (ClassFileReader::FormatError  &e) {
    std::cout << "Wrong class file format: "  << e.what() << "\n";
  }

  std::string ErrorMessage;
  if (!NewClass->verify(ErrorMessage))
    std::cout << "Failed class verification: " + ErrorMessage;

  NewClass->print(std::cout);

  return 0;
}
