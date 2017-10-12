
#ifndef ICP_ENVIRONMENT_H
#define ICP_ENVIRONMENT_H

#include "StackFrame.h"

namespace Verifier {

class JavaClass;
class JavaMethod;
class ConstantPool;

// Represents current verification state
class Environment {
public:
  Environment(
    const ConstantPool &CP,
    const JavaClass &OwnerClass,
    const JavaMethod &Method,
    StackFrame CurrentFrame):
      CP(CP),
      OwnerClass(OwnerClass),
      Method(Method),
      CurrentFrame(std::move(CurrentFrame))
    {
      ;
    }

  const auto &getCP() const { return CP; }
  const auto &getOwner() const { return OwnerClass; }
  const auto &getMethod() const { return Method; }

  auto &getFrame() { return CurrentFrame; }
  const auto &getFrame() const { return CurrentFrame; }

private:
  const ConstantPool &CP;
  const JavaClass &OwnerClass;
  const JavaMethod &Method;
  StackFrame CurrentFrame;
};

}


#endif //ICP_ENVIRONMENT_H
