//
// This class represents constant pool for the single java class.
// It is immutable and only way to construct it is by using the
// ConstantPoolBuilder class.
//

#ifndef IJVM_CONSTANTPOOL_H
#define IJVM_CONSTANTPOOL_H

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

  typedef std::unique_ptr<ConstantPoolRecords::Record> CellType;
  typedef CellType& CellReference;

  typedef std::vector<CellType> RecordTable;

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
    const RecordType *TypedRes = dynamic_cast<const RecordType*>(&Res);
    assert(TypedRes != nullptr);
    return *TypedRes;
  }

  // Get record at the index 'Idx' with a type 'RecordType'.
  // \returns Record at the index 'Idx' or fails an assertion if record doesn't
  //          exist.
  // \returns Null pointer if record has unexpected type.
  template<class RecordType>
  const RecordType *getAsOrNull(IndexType Idx) const {
    const ConstantPoolRecords::Record &Res = get(Idx);
    return dynamic_cast<const RecordType*>(&Res);
  }

  // Get number of records (including dummy empty ones).
  SizeType numRecords() const {
    return static_cast<SizeType>(getRecordTable().size());
  }

  // Check that this constant pool is valid.
  // \returns true if constant pool is valid, false otherwise.
  bool verify(std::string &ErrorMessage) const {
    for (IndexType i = 0; i < numRecords(); ++i) {
      if (getRecordTable()[i] == nullptr) {
        ErrorMessage = "Unallocated record at index " + std::to_string(i + 1);
        return false;
      }
      if (!getRecordTable()[i]->isValid()) {
        ErrorMessage = "Invalid record at index " + std::to_string(i + 1);
        return false;
      }
    }

    return true;
  }

  // Convenience overload of the 'verify' function. Makes it easier to use in
  // asserts.
  // \returns true if constant pool is in a valid state, false otherwise.
  bool verify() const {
    std::string Temp;
    return verify(Temp);
  }

  // Print contents of this constant pool
  void print(std::ostream &Out) const {
    for (IndexType Idx = 1; Idx <= numRecords(); ++Idx) {
      Out << "#" << Idx << " = ";
      get(Idx).print(Out);
    }
  }

  // Checks if index is valid index into this constant pool. Expects 1 based
  // index.
  bool isValidIndex(IndexType Idx) const {
    Idx = toZeroBasedIndex(Idx);
    return Idx >= 0 && Idx < numRecords();
  }

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
  typedef ConstantPool::IndexType IndexType;
  typedef ConstantPool::SizeType SizeType;

  typedef ConstantPool::CellType CellType;
  typedef ConstantPool::CellReference CellReference;
  typedef ConstantPool::RecordTable RecordTable;

public:
  explicit ConstantPoolBuilder(SizeType NumRecords) {
    ConstantPoolUnderConstruction.reset(new ConstantPool(NumRecords));
  }

  CellReference getCellReference(IndexType Idx) const {
    assert(isValid());
    assert(isValidIndex(Idx));
    Idx = ConstantPool::toZeroBasedIndex(Idx);
    return getRecordTable()[Idx];
  }

  void set(IndexType Idx, CellType &&NewRecord) {
    assert(isValid());
    assert(isValidIndex(Idx));
    Idx = ConstantPool::toZeroBasedIndex(Idx);
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

#endif //IJVM_CONSTANTPOOL_H
