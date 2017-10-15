//
//
//

#ifndef ICP_ITERATORS_H
#define ICP_ITERATORS_H

#include <type_traits>
#include <iterator>

namespace Utils {

// This class wraps around random access iterator for the container with smart
// pointers and unwraps each element using .get() function.
// For example:
//   std::vector<std::unique_ptr<const element_type>>
// Will be iterated as if it was:
//   std::vector<const element_type &>
// This is used to expose containers without exposing ownership semantics.
template<class IteratorType>
class SmartPtrIterator {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename IteratorType::value_type::element_type;
  using reference = value_type&;
  using pointer = typename IteratorType::value_type::pointer;
  using difference_type = typename IteratorType::difference_type;

public:
  SmartPtrIterator() = default;

  explicit SmartPtrIterator(IteratorType It): It(It) {
    ;
  }

  reference operator*() const {
    assert(It->get() != nullptr);
    return *It->get();
  }
  pointer operator->() const {
    assert(It->get() != nullptr);
    return It->get();
  }

  SmartPtrIterator& operator++() {
    It++;
    return *this;
  }

  SmartPtrIterator operator++(int) {
    SmartPtrIterator Old(*this);
    It++;
    return Old;
  }

  SmartPtrIterator &operator+=(difference_type n) {
    It += n;
    return *this;
  }

  SmartPtrIterator operator+(difference_type n) const {
    SmartPtrIterator Temp(*this);
    return Temp += n;
  }

  SmartPtrIterator& operator--() {
    It--;
    return *this;
  }

  SmartPtrIterator operator--(int) {
    SmartPtrIterator Old(*this);
    It--;
    return Old;
  }

  SmartPtrIterator &operator-=(difference_type n) {
    return *this += -n;
  }

  SmartPtrIterator operator-(difference_type n) const {
    SmartPtrIterator Temp(*this);
    return Temp -= n;
  }

  difference_type operator-(SmartPtrIterator Other) const {
    return It - Other.It;
  }

  reference operator[](std::size_t Idx) const {
    return It[Idx];
  }

  bool operator<(SmartPtrIterator Other) const {
    return It < Other.It;
  }

  bool operator>(SmartPtrIterator Other) const {
    return *this < Other;
  }

  bool operator>=(SmartPtrIterator Other) {
    return !(*this < Other);
  }

  bool operator<=(SmartPtrIterator Other) {
    return !(*this > Other);
  }

  bool operator==(SmartPtrIterator Other) const {
    return It == Other.It;
  }

  bool operator!=(SmartPtrIterator Other) const {
    return !(*this == Other);
  }

private:
  IteratorType It;

};

}


#endif //ICP_ITERATORS_H
