///
/// Class manager implementation
///

#include "ClassManager.h"

#include "JavaTypes/JavaClass.h"
#include "Verifier/Verifier.h"
#include "SlowInterpreter/SlowInterpreter.h"
#include "ClassFileReader/ClassFileReader.h"
#include "CD/Parser.h"

#include <fstream>

using namespace Runtime;
using namespace JavaTypes;

// Overall loading scheme:
// CM.loadClass -> Loader.loadClass -> (create stream, CM.defineClass(*this)) -> (Loader.deriveClass(), record init and deref class)

const JavaTypes::JavaClass &ClassManager::getClass(
    const Utf8String &Name, const ClassLoader *ILoader/* = nullptr*/) {

  // Choose bootstrap loader by default
  if (ILoader == nullptr) {
    ILoader = getBootstrapLoader();
  }

  // If we already loaded such class - just return it.
  if (const auto *meta_info = getMetaInfoForInitLoader(Name, *ILoader))
    return *meta_info->Class;

  // Otherwise call a class loader to find and define this class
  // It will callback into class manager in order to properly register this class.
  const auto &Class = ILoader->loadClass(Name, *this);

  // Register this class as it's initiating loader
  ClassesInitLoaders[std::make_pair(Class.getClassName(), ILoader)] =
      &getMetaInfoForClass(Class);

  // TODO: This is temporary measure due to the fact that some times class name
  // might not be the same as the file from which it was loaded.
  ClassesInitLoaders[std::make_pair(Name, ILoader)] = &getMetaInfoForClass(Class);
  return Class;
}

Runtime::ClassObject &ClassManager::getClassObject(
    const JavaTypes::JavaClass &Class) {

  auto &meta_info = getMetaInfoForClass(Class);

  // If we already have this object - just return it. Also prevent recursive
  // initialization.
  if (meta_info.State == ClassMetaInfo::INITIALIZED ||
      meta_info.State == ClassMetaInfo::INIT_IN_PROGRESS) {
    assert(meta_info.Object); // should have this object
    return *meta_info.Object;
  }

  // Verify class (throws VerificationError)
  Verifier::verify(Class);

  // Prepare. Happens automatically in the ClassObject constructor
  meta_info.Object = std::unique_ptr<ClassObject>(new ClassObject(Class));

  // Initialize the object
  meta_info.State = ClassMetaInfo::INIT_IN_PROGRESS;
  if (const auto *method = meta_info.Object->getMethod("<clinit>"))
    SlowInterpreter::interpret(*method, {}, *this);
  meta_info.State = ClassMetaInfo::INITIALIZED;

  return *meta_info.Object;
}

const ClassLoader *ClassManager::getDefLoader(
    const JavaTypes::JavaClass &Class) const {

  return &getMetaInfoForClass(Class).DefLoader;
}

JavaTypes::JavaClass &ClassManager::defineClass(
    const Utf8String &Name, std::istream &Bytes, const ClassLoader &DefLoader) {

  // This should always be a new class
  if (getMetaInfoForInitLoader(Name, DefLoader))
    throw LinkageError("Class " + Name + " already loaded");

  // Parse the class (throws in case of an error)
  auto Class = DefLoader.deriveClass(Bytes);
  auto RealName = Class->getClassName();
  // TODO: Check class name
  //assert(RealName == Name);

  // Record the new class
  ClassMetaInfo meta_info{
      DefLoader, std::move(Class), nullptr, ClassMetaInfo::LOADED};

  auto It = Classes.insert(std::make_pair(RealName, std::move(meta_info)));
  ClassesInitLoaders[std::make_pair(RealName, &DefLoader)] = &It->second;

  return *It->second.Class;
}

const ClassManager::ClassMetaInfo *ClassManager::getMetaInfoForInitLoader(
    const Utf8String &Name, const ClassLoader &ILoader) const {

  auto It = ClassesInitLoaders.find(std::make_pair(Name, &ILoader));
  if (It == ClassesInitLoaders.end())
    return nullptr;
  return It->second;
}

const ClassManager::ClassMetaInfo &ClassManager::getMetaInfoForClass(
    const JavaTypes::JavaClass &Class) const {

  auto [beg, end] = Classes.equal_range(Class.getClassName());

  for (auto it = beg; it != end; ++it) {
    if (it->second.Class.get() == &Class)
      return it->second;
  }

  assert(false); // class should be loaded
  return beg->second;
}

ClassManager::ClassMetaInfo &ClassManager::getMetaInfoForClass(
    const JavaTypes::JavaClass &Class) {
  return const_cast<ClassManager::ClassMetaInfo&>(
      const_cast<const ClassManager*>(this)->getMetaInfoForClass(Class));
}

namespace {
class BootstrapLoader: public ClassLoader {
  virtual JavaTypes::JavaClass &loadClass(
      const Utf8String &Name, ClassManager &CM) const override {

    // Hope that class is on cwd.
    // This is not conformant with the spec but who cares.
    std::ifstream file(Name + ".class");
    if (!file)
      throw ClassNotFoundException("Can't find class " + Name);

    return CM.defineClass(Name, file, *this);
  }

  virtual std::unique_ptr<JavaTypes::JavaClass> deriveClass(
      std::istream &Bytes) const override {
    return ClassFileReader::loadClassFromStream(Bytes);
  }
};

class TestLoader: public BootstrapLoader {
  virtual JavaTypes::JavaClass &loadClass(
      const Utf8String &Name, ClassManager &CM) const override {

    std::ifstream file(Name + ".cd");
    if (!file)
      throw ClassNotFoundException("Can't find class " + Name);

    return CM.defineClass(Name, file, *this);
  }

  virtual std::unique_ptr<JavaTypes::JavaClass> deriveClass(
      std::istream &Bytes) const override {
    return CD::parseFromStream(Bytes);
  }
};
}

const ClassLoader *Runtime::getBootstrapLoader() {
  static BootstrapLoader l;
  return &l;
}

const ClassLoader *Runtime::getTestLoader() {
  static TestLoader l;
  return &l;
}
