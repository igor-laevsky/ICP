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
    explicit IteratorImpl(UnderlyingItType NewIt):
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
      return &It->second;
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

  // No copies, no moves
  BciMap(const BciMap&) = delete;
  BciMap &operator=(const BciMap&) = delete;
  BciMap(BciMap&&) = delete;
  BciMap &operator=(BciMap&&) = delete;

  /// Insert new element into the map.
  /// \return true if new element was added, false otherwise.
  bool insert(BciType Bci, T&& El) {
    return storage().insert(std::make_pair(Bci, std::forward<T>(El))).second;
  }

  /// Handful accessors mirroring default std containers
  std::size_t size() const { return storage().size(); }
  bool empty() const { return storage().empty(); }

  iterator begin() { return iterator(storage().begin()); }
  const_iterator begin() const { return const_iterator(storage().begin()); }
  const_iterator cbegin() const { return const_iterator(storage().begin()); }

  iterator end() { return iterator(storage().end()); }
  const_iterator end() const { return const_iterator(storage().end()); }
  const_iterator cend() const { return const_iterator(storage().end()); }

  iterator offsetTo(iterator It, BciOffsetType Off) {
    return offsetToImpl(It, Off, begin(), end());
  }

  const_iterator offsetTo(const_iterator It, BciOffsetType Off) const {
    return offsetToImpl(It, Off, begin(), end());
  }

  const_iterator findAtBci(BciType Bci) const {
    return const_iterator(storage().find(Bci));
  }

private:
  // Template function in order to support const and non-const iterators.
  template<typename ItType, typename OwnerIt>
  ItType offsetToImpl(ItType It, BciOffsetType Off, OwnerIt Beg, OwnerIt End) const {
    auto ret = It;
    auto target_bci = It.getBci() + Off;

    if (Off == 0 || It == End)
      return ret;

    auto real_end = Off > 0 ? End : Beg;

    while (ret != real_end) {
      if (ret.getBci() == target_bci)
        break;

      if (Off > 0) {
        ret++;
      } else {
        ret--;
      }
    }

    if (ret != End && ret.getBci() == target_bci)
      return ret;
    return End;
  }

private:
  StorageType &storage() { return Storage; }
  const StorageType &storage() const { return Storage; }

private:
  StorageType Storage;
};

}

#endif //ICP_BCIMAP_H
