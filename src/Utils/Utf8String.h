//
// This class should be used to support unicode strings from the java class
// file.
//

#ifndef ICP_UTF8STRING_H
#define ICP_UTF8STRING_H

#include <string>

namespace Utils {
  // TODO: This is quite imperfect solution but should be fine for now.
  using Utf8String = std::string;
}

#endif //ICP_UTF8STRING_H
