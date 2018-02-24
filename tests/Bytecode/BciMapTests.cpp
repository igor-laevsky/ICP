///
/// Tests for the BciMap
///

#include "catch.hpp"

#include "Bytecode/BciMap.h"

using namespace Bytecode;

TEST_CASE("Basic BciMap", "[Bytecode][BciMap]") {
  BciMap<char> M;

  REQUIRE(M.begin() == M.end());
  REQUIRE(M.size() == 0);
  REQUIRE(M.empty());

  REQUIRE(M.insert(0, 'a'));
  REQUIRE(M.insert(1, 'b'));
  REQUIRE(M.insert(4, 'c'));
  REQUIRE(M.insert(6, 'd'));
  REQUIRE(M.insert(10, 'e'));
  // No duplicate insertions
  REQUIRE_FALSE(M.insert(10, 'u'));

  REQUIRE(M.begin() != M.end());
  REQUIRE(M.size() == 5);
  REQUIRE(!M.empty());


  auto It = M.cbegin();

  REQUIRE(It.getBci() == 0);
  REQUIRE(*It == 'a');
  REQUIRE(M.findAtBci(0) == M.cbegin());
  It++;
  REQUIRE(It.getBci() == 1);
  REQUIRE(*It == 'b');
  It++;
  REQUIRE(It.getBci() == 4);
  REQUIRE(*It == 'c');
  ++It;
  REQUIRE(It.getBci() == 6);
  REQUIRE(*It == 'd');
  REQUIRE(M.findAtBci(6) == It);
  It++;
  REQUIRE(It.getBci() == 10);
  REQUIRE(*It == 'e');
  ++It;
  REQUIRE(It == M.cend());

  It = M.cbegin();

  It = M.offsetTo(It, 1);
  REQUIRE(It.getBci() == 1);
  REQUIRE(*It == 'b');
  It = M.offsetTo(It, -1);
  REQUIRE(It.getBci() == 0);
  REQUIRE(*It == 'a');

  It = M.offsetTo(It, 10);
  REQUIRE(It.getBci() == 10);
  REQUIRE(*It == 'e');

  It = M.offsetTo(It, -4);
  REQUIRE(It.getBci() == 6);
  REQUIRE(*It == 'd');

  REQUIRE(M.offsetTo(It, -1) == M.cend());
  It = M.offsetTo(It, -5);
  REQUIRE(It.getBci() == 1);
  REQUIRE(*It == 'b');

  It = M.offsetTo(It, 0);
  REQUIRE(It.getBci() == 1);
  REQUIRE(*It == 'b');

  // No offset past the beginning of the map.
  REQUIRE(M.offsetTo(It, -10) == M.cend());
  // Can't offset past the end of a map.
  REQUIRE(M.offsetTo(It, 100) == M.cend());

  REQUIRE(M.findAtBci(100) == M.cend());
}

TEST_CASE("Const BciMap", "[Bytecode][BciMap]") {
  BciMap<char> M;

  auto &P = M;
  const auto &CP = M;

  auto ItP = P.begin();
  BciMap<char>::const_iterator ItCP = CP.begin();

  // Non const object produces non const iterators
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(*ItP)>>);
  // Const object produces const iterators
  static_assert(std::is_const_v<std::remove_reference_t<decltype(*ItCP)>>);

  // This is compile time test
  REQUIRE(true);
}

TEST_CASE("Unique ptr BciMap", "[Bytecode][BciMap]") {
  BciMap<std::unique_ptr<char>> M;

  M.insert(0, std::make_unique<char>('a'));
  M.insert(5, std::make_unique<char>('b'));

  REQUIRE(M.size() == 2);
  REQUIRE(!M.empty());

  REQUIRE(M.begin().getBci() == 0);

  auto p = std::make_unique<char>('e');
  //M.insert(10, p); // fail compilation
  M.insert(10, std::move(p)); // ok
}
