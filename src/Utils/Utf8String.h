//
// This class should be used to support unicode strings from the java class
// file.
//

#ifndef ICP_UTF8STRING_H
#define ICP_UTF8STRING_H

#include <string>

//namespace Utils {
  // TODO: This is quite imperfect solution but should be fine for now.
  using Utf8String = std::string;

  inline bool starts_with(const Utf8String &In, const Utf8String &Prefix) {
    if (In.length() < Prefix.length())
      return false;

    for (std::size_t i = 0; i < Prefix.length(); ++i)
      if (Prefix[i] != In[i])
        return false;
    return true;
  }
//}

#endif //ICP_UTF8STRING_H
