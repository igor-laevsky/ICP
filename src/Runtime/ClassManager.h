///
/// Class which manages all java classes. It is responsible for loading, linking
/// and initializing classes at the right moments of time.
///

#ifndef ICP_CLASSMANAGER_H
#define ICP_CLASSMANAGER_H

#include "Runtime/Objects.h"
#include "JavaTypes/JavaTypesFwd.h"

#include <map>

namespace Runtime {

class ClassNotFoundException: public std::runtime_error {
  using runtime_error::runtime_error;
};
class LinkageError: public std::runtime_error {
  using runtime_error::runtime_error;
};

class ClassManager;

class ClassLoader {
public:
  ClassLoader() = default;
  virtual ~ClassLoader() = default;

  virtual JavaTypes::JavaClass &loadClass(
      const Utf8String &Name, ClassManager &CM) const = 0;

  virtual std::unique_ptr<JavaTypes::JavaClass> deriveClass(
      std::istream &Bytes) const = 0;
};

const ClassLoader *getBootstrapLoader();
const ClassLoader *getTestLoader();


// TODO: This should be thread safe and it's not
class ClassManager final {
public:
  // Creates empty class manager
  ClassManager() = default;

  // No copies
  ClassManager(const ClassManager&) = delete;
  ClassManager &operator=(const ClassManager&) = delete;
  // No moves
  ClassManager(ClassManager&&) = delete;
  ClassManager &operator=(ClassManager&&) = delete;

  // Load new class or return already existing one.
  // \throws ClassNotFoundException
  // \throws LinkageError
  const JavaTypes::JavaClass &getClass(
      const Utf8String &Name, const ClassLoader *ILoader = nullptr);

  // Create object for the loaded class.
  // This involves verification. preparation and initialization.
  // \throws VerificationError
  Runtime::ClassObject &getClassObject(const JavaTypes::JavaClass &Class);

  const ClassLoader *getDefLoader(const JavaTypes::JavaClass &Class) const;

  // Helper method for the class loaders.
  // \throws Various class parsing errors depending on the parsing method
  JavaTypes::JavaClass &defineClass(
      const Utf8String &Name, std::istream &Bytes, const ClassLoader &DefLoader);

private:
  struct ClassMetaInfo {
    const ClassLoader &DefLoader;
    std::unique_ptr<JavaTypes::JavaClass> Class;
    std::unique_ptr<ClassObject> Object;
  };

private:
  // \returns null if no information was found
  const ClassMetaInfo *getMetaInfoForInitLoader(
      const Utf8String &Name, const ClassLoader &ILoader) const;

  // Always succeeds since class must have been loaded
  const ClassMetaInfo &getMetaInfoForClass(
      const JavaTypes::JavaClass &Class) const;
  ClassMetaInfo &getMetaInfoForClass(const JavaTypes::JavaClass &Class);

private:
  std::multimap<Utf8String, ClassMetaInfo> Classes;

  std::map<std::pair<Utf8String, const ClassLoader*>, const ClassMetaInfo*>
      ClassesInitLoaders;
};



class OldClassManager {
public:
  friend OldClassManager &getClassManager();

  OldClassManager(const OldClassManager&) = delete;
  OldClassManager &operator=(const OldClassManager&) = delete;

  void registerClass(ClassObject &CO);

  ClassObject &getClassObject(const Utf8String &Name);

  // Mainly required for the sane testing. Need to avoid it once this class
  // becomes more advanced.
  void reset();

private:
  OldClassManager() = default;

private:
  std::map<Utf8String, ClassObject*> Classes;
};

OldClassManager &getClassManager();

}

#endif //ICP_CLASSMANAGER_H
