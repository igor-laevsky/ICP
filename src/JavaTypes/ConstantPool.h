//
// This class represents constant pool for the single java class.
// It is immutable and only way to construct it is by using the
// ConstantPoolBuilder class.
//

#ifndef ICP_CONSTANTPOOL_H
#define ICP_CONSTANTPOOL_H

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <ostream>

namespace JavaTypes {

class ConstantPool;

namespace ConstantPoolRecords {

// Base abstract class for all constant pool records
class Record {
public:
  virtual ~Record() = default;

  Record() = default;

  // No copying
  Record(const Record &) = delete;
  Record &operator=(const Record &) = delete;

  virtual bool isValid() const = 0;

  virtual void print(std::ostream &) const = 0;
};

}

class ConstantPool final {
public:
  typedef uint16_t IndexType;
  typedef uint16_t SizeType;

  template<class T> using CellType = std::unique_ptr<T>;
  template<class T> using CellReference = const CellType<T>&;

  typedef std::vector<CellType<ConstantPoolRecords::Record>> RecordTable;

public:
  // Get record at the index 'Idx'.
  // \returns Record at the index 'Idx' or fails an assertion if record doesn't
  //          exist.
  const ConstantPoolRecords::Record &get(IndexType Idx) const {
    assert(isValidIndex(Idx));
    Idx = ConstantPool::toZeroBasedIndex(Idx);
    assert(getRecordTable()[Idx] != nullptr);
    return *getRecordTable()[Idx];
  }

  // Get record at the index 'Idx' with a type 'RecordType'.
  // \returns Record at the index 'Idx' or fails an assertion if record doesn't
  //          exist or has unexpected type.
  template<class RecordType>
  const RecordType &getAs(IndexType Idx) const {
    const ConstantPoolRecords::Record &Res = get(Idx);
    auto *TypedRes = dynamic_cast<const RecordType*>(&Res);
    assert(TypedRes != nullptr);
    return *TypedRes;
  }

  // Get record at the index 'Idx' with a type 'RecordType'.
  // \returns Record at the index 'Idx' or nullptr if index is out of range
  //          or record has the wrong type.
  // \returns Null pointer if record has unexpected type.
  template<class RecordType>
  const RecordType *getAsOrNull(IndexType Idx) const {
    if (!isValidIndex(Idx))
      return nullptr;

    const ConstantPoolRecords::Record &Res = get(Idx);
    return dynamic_cast<const RecordType*>(&Res);
  }

  // Checks if the record has given type.
  // \returns true if record exists and has desired type. false if record
  //          exists but has the wrong type or doesn't exist.
  template<class RecordType>
  bool isA(IndexType Idx) const {
    if (!isValidIndex(Idx))
      return false;
    const auto &Res = get(Idx);
    return dynamic_cast<const RecordType*>(&Res) != nullptr;
  }

  // Get number of records (including dummy empty ones).
  SizeType numRecords() const {
    return static_cast<SizeType>(getRecordTable().size());
  }

  // Checks if index is valid index into this constant pool. Expects 1 based
  // index.
  bool isValidIndex(IndexType Idx) const {
    return Idx >= 1 && Idx <= numRecords();
  }

  // Check that this constant pool is valid.
  // \returns true if constant pool is valid, false otherwise. ErrorMessage
  // will remain unchanged when constant pool is valid or will contain
  // diagnostic message if any verification errors were discovered.
  bool verify(std::string &ErrorMessage) const;

  // Convenience overload of the 'verify' function. Makes it easier to use in
  // asserts.
  // \returns true if constant pool is in a valid state, false otherwise.
  bool verify() const;

  // Print contents of this constant pool
  void print(std::ostream &Out) const;

  // No copying
  ConstantPool(const ConstantPool &) = delete;
  ConstantPool &operator=(const ConstantPool &) = delete;

private:
  // Only possible to construct through the ConstantPoolBuilder.
  explicit ConstantPool(SizeType NumRecords): Records(NumRecords) {
    ;
  }

  const RecordTable &getRecordTable() const {
    return Records;
  }

  // Indexes used in the class file are 1 based, but internally we use
  // zero based indexes. All interface functions expect 1 based indexes.
  static IndexType toZeroBasedIndex(IndexType Idx) {
    return Idx - (IndexType)1;
  }

private:
  RecordTable Records;

  friend class ConstantPoolBuilder;
};

class ConstantPoolBuilder final {
public:
  using IndexType = ConstantPool::IndexType;
  using SizeType = ConstantPool::SizeType;

  template<class T> using CellType = ConstantPool::CellType<T>;
  template<class T> using CellReference = ConstantPool::CellReference<T>;
  using RecordTable = ConstantPool::RecordTable;

  template<class T> static constexpr bool IsCPRecord =
      std::is_base_of_v<ConstantPoolRecords::Record, T>;

public:
  explicit ConstantPoolBuilder(SizeType NumRecords) {
    ConstantPoolUnderConstruction.reset(new ConstantPool(NumRecords));
  }

  template<class T, class X = std::enable_if_t<IsCPRecord<T>>>
  CellReference<T> getCellReference(IndexType Idx) const {
    assert(isValid());
    assert(isValidIndex(Idx));
    Idx = ConstantPool::toZeroBasedIndex(Idx);
    return reinterpret_cast<CellReference<T>>(getRecordTable()[Idx]);
  }

  template<class T, class X = std::enable_if_t<IsCPRecord<T>>>
  void set(IndexType Idx, CellType<T> &&NewRecord) {
    assert(isValid());
    assert(isValidIndex(Idx));
    Idx = ConstantPool::toZeroBasedIndex(Idx);

    assert(getRecordTable()[Idx] == nullptr); // only assign once
    getRecordTable()[Idx] = std::move(NewRecord);
  }

  std::unique_ptr<ConstantPool> createConstantPool() {
    assert(isValid());
    return std::move(ConstantPoolUnderConstruction);
  }

  // Checks if this builder is still valid.
  bool isValid() const {
    return ConstantPoolUnderConstruction != nullptr;
  }

private:
  RecordTable &getRecordTable() const {
    return ConstantPoolUnderConstruction->Records;
  }

  bool isValidIndex(IndexType Idx) const {
    return ConstantPoolUnderConstruction->isValidIndex(Idx);
  }

private:
  std::unique_ptr<ConstantPool> ConstantPoolUnderConstruction;
};

}

#endif //ICP_CONSTANTPOOL_H
