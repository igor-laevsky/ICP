///
/// Implementation of the CD parser. There is noticeable simmetry with binary
/// class file reader so if you change something here, consider updating class
/// file reader.
///

#include "Parser.h"

#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <variant>

#include "Lexer.h"
#include "JavaTypes/JavaClass.h"
#include "JavaTypes/JavaMethod.h"
#include "JavaTypes/ConstantPool.h"
#include "JavaTypes/ConstantPoolRecords.h"

using namespace CD;
using namespace JavaTypes;

// Helper function to avoid repeating patters:
//  if (!Lex.consume(Tok)) throw ...
static const Token &consumeOrThrow(const Token &Tok, Lexer &Lex) {
  const auto &Res = Lex.consume(Tok);
  if (!Res)
    throw ParserError("Expected " + to_string(Tok));

  return *Res;
}

// Helper function. Parses #<number> or returns nothing.
static std::optional<ConstantPool::IndexType> tryParseCPIndex(Lexer &Lex) {
  if (!Lex.consume(Token::Sharp))
    return std::nullopt;

  const auto &Res = consumeOrThrow(Token::Num(), Lex);
  return static_cast<ConstantPool::IndexType>(std::stoi(Res.getData()));
}

// Helper function. Parses CP index or throws ParserError.
static ConstantPool::IndexType parseCPIndex(Lexer &Lex) {
  const auto &Ret = tryParseCPIndex(Lex);
  if (!Ret)
    throw ParserError("Expected constant pool index");

  return *Ret;
}

// Helper function. Looks for the specified string in the constant pool.
// This is inefficient and should be replaced with the proper symbol table.
// \returns Pointer to the cp record or null if nothing found.
static const ConstantPoolRecords::Utf8 *findStringInCP(
    const std::string &Target, const ConstantPool &CP) {

  for (ConstantPool::IndexType Idx = 1; Idx <= CP.numRecords(); ++Idx) {
    if (const auto *Rec = CP.getAsOrNull<ConstantPoolRecords::Utf8>(Idx)) {
      if (Rec->getValue() == Target)
        return Rec;
    }
  }

  return nullptr;
}

static std::unique_ptr<ConstantPool> parseConstantPool(Lexer &Lex) {
  if (!Lex.consume(Token::Keyword("constant_pool")))
    throw ParserError("Expected constant_pool as a first member of the class");
  if (!Lex.consume(Token::LBrace))
    throw ParserError("Expected LBrace");

  // There are number of things we need to precompute before we can start the
  // actual constant pool creation process, so we follow a multistep process:
  // 1. Parse everything into temporary data structures
  // 2. Assign cp indexes for the unique strings
  // 3. Create constant pool


  // Temporary data structures to store pre-parsed constant pool
  struct Record {
    std::string Type = "";

    using ArgType =
      std::variant<ConstantPool::IndexType, std::string>;
    std::vector<ArgType> Args;
  };
  std::map<ConstantPool::IndexType, Record> ParsedRecords;
  ConstantPool::IndexType MaxCPIdx = 0;
  // Collect free standing strings with no indexes assigned
  std::map<std::string, ConstantPool::IndexType> StringToIdx;

  // Parse constant pool into the structures defined above
  // Compute maximal pre defined index along the way
  //

  while (!Lex.isNext(Token::RBrace)) {
    if (Lex.isNext(Token::Num())) {
      // Parse "<number>: <id> <args>"

      auto Idx = static_cast<ConstantPool::IndexType>(
          std::stoi(Lex.consume().getData()));
      if (Idx <= 0)
        throw ParserError("Constant pool index should be greater than zero");
      if (ParsedRecords.count(Idx) > 0)
        throw ParserError(
            "Duplicated constant pool index found: " + std::to_string(Idx));

      MaxCPIdx = std::max(MaxCPIdx, Idx);
      consumeOrThrow(Token::Colon, Lex);

      Record NewRec;
      NewRec.Type = consumeOrThrow(Token::Id(), Lex).getData();

      bool hasArgs = true;
      while (hasArgs) {
        if (const auto &ArgIdx = tryParseCPIndex(Lex)) {
          NewRec.Args.emplace_back(*ArgIdx);
        } else if (const auto &Tok = Lex.consume(Token::String())) {
          NewRec.Args.emplace_back(Tok->getData());
          StringToIdx[Tok->getData()] = 0;
        } else {
          hasArgs = false;
        }
      }

      ParsedRecords[Idx] = std::move(NewRec);

    } else if (Lex.consume(Token::Keyword("auto"))) {
      // Parse "auto: <string>"

      consumeOrThrow(Token::Colon, Lex);
      const auto &Str = consumeOrThrow(Token::String(), Lex).getData();
      StringToIdx[Str] = 0;
    } else {
      throw ParserError("Expected RBrace at the end of constant pool");
    }
  }


  // Allocate constant pool indexes for the "free standing" strings
  //

  for (const auto &[Str, Idx]: StringToIdx) {
    assert(Idx == 0); // no indexes should be assigned
    MaxCPIdx++;
    StringToIdx[Str] = MaxCPIdx;
  }

  // Actually build constant pool
  //

  ConstantPoolBuilder Builder(MaxCPIdx);

  // Helper function which receives variant holding index or string and
  // converts it to the real constant pool index.
  auto GetIdxForArg = [&] (const Record::ArgType &Arg) {
    if (std::holds_alternative<ConstantPool::IndexType>(Arg))
      return std::get<ConstantPool::IndexType>(Arg);

    assert(std::holds_alternative<std::string>(Arg));

    const auto &RetIdx = StringToIdx[std::get<std::string>(Arg)];
    assert(RetIdx != 0); // should have assigned indexes for all strings
    return RetIdx;
  };

  // Create non string records
  for (const auto &[Idx, Rec]: ParsedRecords) {
    std::unique_ptr<ConstantPoolRecords::Record> NewCPRec(nullptr);

    if (Rec.Type == "ClassInfo") {
      if (Rec.Args.size() != 1)
        throw ParserError("ClassInfo record should have exactly one argument");

      NewCPRec = std::make_unique<ConstantPoolRecords::ClassInfo>(
          Builder.getCellReference(GetIdxForArg(Rec.Args[0])));
    } else if (Rec.Type == "NameAndType") {
      if (Rec.Args.size() != 2)
        throw ParserError("NameAndType record should have exactly two arguments");

      NewCPRec = std::make_unique<ConstantPoolRecords::NameAndType>(
          Builder.getCellReference(GetIdxForArg(Rec.Args[0])),
          Builder.getCellReference(GetIdxForArg(Rec.Args[1])));
    } else if (Rec.Type == "MethodRef") {
      if (Rec.Args.size() != 2)
        throw ParserError("MethodRef record should have exactly two arguments");

      NewCPRec = std::make_unique<ConstantPoolRecords::MethodRef>(
          Builder.getCellReference(GetIdxForArg(Rec.Args[0])),
          Builder.getCellReference(GetIdxForArg(Rec.Args[1])));
    }

    assert(Idx > 0); // all indexes should have been assigned
    Builder.set(Idx, std::move(NewCPRec));
  }

  // Create string records
  for (const auto &[Str, Idx]: StringToIdx) {
    assert(Idx > 0); // all indexes should have been assigned
    Builder.set(Idx, std::make_unique<ConstantPoolRecords::Utf8>(Str));
  }

  consumeOrThrow(Token::RBrace, Lex);

  // Final step - create real constant pool and verify it
  std::unique_ptr<ConstantPool> CP = Builder.createConstantPool();

  std::string ErrorMessage;
  if (!CP->verify(ErrorMessage))
    throw ParserError(
        "Failed constant pool verification with message: " + ErrorMessage);

  return CP;
}

static JavaMethod::CodeOwnerType parseBytecode(Lexer &Lex) {

  consumeOrThrow(Token::Keyword("bytecode"), Lex);
  consumeOrThrow(Token::LBrace, Lex);

  JavaMethod::CodeOwnerType Ret;
  while (!Lex.isNext(Token::RBrace)) {
    const std::string &Name = consumeOrThrow(Token::Id(), Lex).getData();
    // Only single indexed instructions for now.
    // Sometimes instructions have index but it's not a constant pool index.
    // Treat them similarly since there is no practical difference.
    const auto &IdxOpt = tryParseCPIndex(Lex);
    const auto Idx = IdxOpt ? static_cast<Bytecode::IdxType>(*IdxOpt) : 0;

    try {
      Ret.push_back(Bytecode::parseFromString(Name, Idx));
    } catch (Bytecode::UnknownBytecode &) {
      throw ParserError("Unable to parse method bytecode");
    }
    assert(Ret.back() != nullptr);
  }

  consumeOrThrow(Token::RBrace, Lex);

  return Ret;
}

static std::unique_ptr<JavaMethod> parseMethod(
    Lexer &Lex,
    const ConstantPool &CP) {

  JavaMethod::MethodConstructorParameters Params;

  consumeOrThrow(Token::Keyword("method"), Lex);

  // Parse name and descriptor
  const std::string &Name = consumeOrThrow(Token::String(), Lex).getData();
  Params.Name = findStringInCP(Name, CP);
  if (Params.Name == nullptr)
    throw ParserError("Method name was not found in constant pool");

  const std::string &Descr = consumeOrThrow(Token::String(), Lex).getData();
  Params.Descriptor = findStringInCP(Descr, CP);
  if (Params.Descriptor == nullptr)
    throw ParserError("Method descriptor was not found in constant pool");

  consumeOrThrow(Token::LBrace, Lex);

  // Parse flags
  consumeOrThrow(Token::Id("Flags"), Lex);
  consumeOrThrow(Token::Colon, Lex);

  Params.Flags = JavaMethod::AccessFlags::ACC_NONE;
  do {
    const std::string &FlagName = consumeOrThrow(Token::Id(), Lex).getData();

    if (FlagName == "public")
      Params.Flags = Params.Flags | JavaMethod::AccessFlags::ACC_PUBLIC;
    else if (FlagName == "static")
      Params.Flags = Params.Flags | JavaMethod::AccessFlags::ACC_STATIC;
    else
      throw ParserError("Unrecognized method access flag");

  } while (Lex.consume(Token::Comma));

  // Parse MaxStack and MaxLocals
  consumeOrThrow(Token::Id("MaxStack"), Lex);
  consumeOrThrow(Token::Colon, Lex);
  Params.MaxStack = static_cast<uint16_t>(
      std::stoi(consumeOrThrow(Token::Num(), Lex).getData()));

  consumeOrThrow(Token::Id("MaxLocals"), Lex);
  consumeOrThrow(Token::Colon, Lex);
  Params.MaxLocals = static_cast<uint16_t>(
      std::stoi(consumeOrThrow(Token::Num(), Lex).getData()));

  // Parse bytecode
  Params.Code = parseBytecode(Lex);

  consumeOrThrow(Token::RBrace, Lex);

  return std::make_unique<JavaMethod>(std::move(Params));
}

static void parseClass(JavaClass::ClassParameters &Params, Lexer &Lex) {
  consumeOrThrow(Token::Keyword("class"), Lex);
  consumeOrThrow(Token::LBrace, Lex);

  // Parse constant pool
  Params.CP = parseConstantPool(Lex);

  // Parse class name
  consumeOrThrow(Token::Id("Name"), Lex);
  consumeOrThrow(Token::Colon, Lex);
  const auto &ClassNameIdx = parseCPIndex(Lex);
  if (!Params.CP->isA<ConstantPoolRecords::ClassInfo>(ClassNameIdx))
    throw ParserError("Class name cp record has unexpected type");
  Params.ClassName =
      &Params.CP->getAs<ConstantPoolRecords::ClassInfo>(ClassNameIdx);

  // Parse super class name
  consumeOrThrow(Token::Id("Super"), Lex);
  consumeOrThrow(Token::Colon, Lex);
  const auto &SuperIdx = parseCPIndex(Lex);
  if (!Params.CP->isA<ConstantPoolRecords::ClassInfo>(SuperIdx))
    throw ParserError("Super class name cp record has unexpected type");
  Params.SuperClass =
      &Params.CP->getAs<ConstantPoolRecords::ClassInfo>(SuperIdx);

  // TODO: Use real access flags
  Params.Flags =
      JavaClass::AccessFlags::ACC_PUBLIC | JavaClass::AccessFlags::ACC_SUPER;

  // Parse methods
  while (Lex.isNext(Token::Keyword("method"))) {
    Params.Methods.push_back(parseMethod(Lex, *Params.CP));
  }

  consumeOrThrow(Token::RBrace, Lex);
}

std::unique_ptr<JavaTypes::JavaClass> CD::parseFromStream(
    std::istream &InputStream) {

  // Lex
  Lexer Lex{std::string(
      std::istreambuf_iterator<char>(InputStream),
      std::istreambuf_iterator<char>())};
  if (!Lex.hasNext())
    throw ParserError("Unexpected empty input");

  // Parse
  JavaClass::ClassParameters Params;

  parseClass(Params, Lex);

  return std::make_unique<JavaClass>(std::move(Params));
}

std::unique_ptr<JavaTypes::JavaClass> CD::parseFromFile(
    const std::string &FileName) {

  std::ifstream File(FileName, std::ios_base::in);
  if (!File)
    throw FileNotFound();

  return parseFromStream(File);
}

std::unique_ptr<JavaTypes::JavaClass> CD::parseFromString(
    const std::string &Input) {

  std::istringstream InputStream(Input);
  assert(InputStream); // What can go wrong?

  return parseFromStream(InputStream);
}
