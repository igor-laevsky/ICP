///
/// Utility class which helps with storing (Bci -> Value) mappings, primary
/// intended for the instruction to bci mapping.
///

#ifndef ICP_BCIMAP_H
#define ICP_BCIMAP_H

#include <map>

#include "Bytecode/BytecodeFwd.h"
#include "Utils/Iterators.h"

namespace Bytecode {

template<typename T>
class BciMap {
private:
  // Iterator for the bci map. On dereference returns contained object constness
  // of which depends on the type of the underlying iterator.
  template<typename UnderlyingItType>
  class IteratorImpl {
  public:
    using value_type = decltype(std::declval<UnderlyingItType>()->second);
    using reference = std::conditional_t<
        Utils::IsConstIter<UnderlyingItType>, const value_type&, value_type&>;
    using pointer = std::conditional_t<
        Utils::IsConstIter<UnderlyingItType>, const value_type*, value_type*>;

  public:
    IteratorImpl() = default;
    IteratorImpl(UnderlyingItType NewIt):
        It(NewIt) {
      ;
    }

    Bytecode::BciType getBci() const { return It->first; }

    IteratorImpl &operator++() {
      It++;
      return *this;
    }

    IteratorImpl operator++(int) {
      IteratorImpl Old(*this);
      It++;
      return Old;
    }

    IteratorImpl &operator--() {
      It--;
      return *this;
    }

    IteratorImpl operator--(int) {
      IteratorImpl Old(*this);
      It--;
      return Old;
    }

    reference operator*() const {
      return It->second;
    }
    pointer operator->() const {
      return std::addressof(It->second);
    }

    bool operator==(const IteratorImpl &Other) const {
      return It == Other.It;
    }
    bool operator!=(const IteratorImpl &Other) const {
      return It != Other.It;
    }


  private:
    UnderlyingItType It;
  };

  // Container must be sorted by it's key (which is bci).
  using StorageType = std::map<BciType, T>;

public:
  using iterator = IteratorImpl<typename StorageType::iterator>;
  using const_iterator = IteratorImpl<typename StorageType::const_iterator>;

public:
  BciMap() = default;

  // No copies
  BciMap(const BciMap&) = delete;
  BciMap &operator=(const BciMap&) = delete;
  // Allow moves
  BciMap(BciMap&&) = default;
  BciMap &operator=(BciMap&&) = default;

  /// Insert new element into the map.
  /// \return true if new element was added, false otherwise.
  bool insert(BciType Bci, T&& El) {
    return storage().insert(std::make_pair(Bci, std::forward<T>(El))).second;
  }

  /// This should be used when inserting instruction in a sorted order.
  /// The order shouldn't be precise and will only affect speed of the operation.
  template <class... Args>
  iterator emplace_back(BciType Bci, Args&&... args) {
    return storage().emplace_hint(storage().end(), Bci, std::forward<Args>(args)...);
  }
  iterator insert_back(BciType Bci,  T&& El) {
    return storage().insert(storage().end(), std::make_pair(Bci, std::forward<T>(El)));
  }

  /// Handful accessors mirroring default std containers
  std::size_t size() const { return storage().size(); }
  bool empty() const { return storage().empty(); }

  iterator begin() { return storage().begin(); }
  const_iterator begin() const { return storage().begin(); }
  const_iterator cbegin() const { return storage().begin(); }

  iterator end() { return storage().end(); }
  const_iterator end() const { return storage().end(); }
  const_iterator cend() const { return storage().end(); }
  
  iterator offsetTo(iterator It, BciOffsetType Off) {
    return storage().find(It.getBci() + Off);
  }
  const_iterator offsetTo(const_iterator It, BciOffsetType Off) const {
    return storage().find(It.getBci() + Off);
  }

  iterator find(BciType Bci) { return storage().find(Bci); }
  const_iterator find(BciType Bci) const { return storage().find(Bci); }

private:
  StorageType &storage() { return Storage; }
  const StorageType &storage() const { return Storage; }

private:
  StorageType Storage;
};

}

#endif //ICP_BCIMAP_H
