//
//
//

#include <type_traits>

#include "catch.hpp"

#include "Utils/Iterators.h"

using namespace Utils;

TEST_CASE("SmartPtrIterator", "[Iterators]") {
  // Ensure that iterator wrapper preserves constness of the pointer elements
  // and that constness of the underlying iterator doesn't matter.

  using VectorIterator = std::vector<std::unique_ptr<int>>::iterator;
  using VectorConstIterator = std::vector<std::unique_ptr<int>>::const_iterator;
  using ConstVectorIterator = std::vector<std::unique_ptr<const int>>::iterator;
  using ConstVectorConstIterator = std::vector<std::unique_ptr<const int>>::const_iterator;

  static_assert(
      !std::is_const<
        std::remove_reference<
          SmartPtrIterator<VectorIterator>::reference>::type>::value);
  static_assert(
      !std::is_const<
          std::remove_reference<
              SmartPtrIterator<VectorConstIterator>::reference>::type>::value);

  static_assert(
      std::is_const<
          std::remove_reference<
              SmartPtrIterator<ConstVectorIterator>::reference>::type>::value);
  static_assert(
      std::is_const<
          std::remove_reference<
              SmartPtrIterator<ConstVectorConstIterator>::reference>::type>::value);

  // This is compile time test
  REQUIRE(true);
}
