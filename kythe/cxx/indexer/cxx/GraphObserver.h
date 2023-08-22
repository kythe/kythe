/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KYTHE_CXX_INDEXER_CXX_GRAPH_OBSERVER_H_
#define KYTHE_CXX_INDEXER_CXX_GRAPH_OBSERVER_H_

/// \file
/// \brief Defines the class kythe::GraphObserver

#include <optional>
#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/log/check.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Lex/Preprocessor.h"
#include "kythe/cxx/common/indexing/KytheCachingOutput.h"
#include "kythe/cxx/common/kythe_metadata_file.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {

// TODO(zarko): Most of the documentation for this interface belongs here.

enum class ProfilingEvent {
  Enter,  ///< A profiling section was entered.
  Exit    ///< A profiling section was left.
};

/// \brief A callback used to report a profiling event.
///
/// Profile events have labels (formatted as lowercase strings with words
/// separated by underscores) and event types.
using ProfilingCallback = std::function<void(const char*, ProfilingEvent)>;

/// \brief Ensures that Enter events are paired with Exit events.
class ProfileBlock {
 public:
  /// \param Callback reporting callback; must outlive `ProfileBlock`
  /// \param SectionName name for the section; must outlive `ProfileBlock`
  ProfileBlock(const ProfilingCallback& Callback, const char* SectionName)
      : Callback(Callback), SectionName(SectionName) {
    Callback(SectionName, ProfilingEvent::Enter);
  }
  ~ProfileBlock() { Callback(SectionName, ProfilingEvent::Exit); }

 private:
  const ProfilingCallback& Callback;
  const char* SectionName;
};

class HashRecorder {
 public:
  virtual void RecordHash(absl::string_view hash, absl::string_view web64hash,
                          absl::string_view original) {}
  virtual ~HashRecorder() = default;
};

class FileHashRecorder : public HashRecorder {
 public:
  explicit FileHashRecorder(absl::string_view path);
  void RecordHash(absl::string_view hash, absl::string_view web64hash,
                  absl::string_view original) override;
  ~FileHashRecorder() override;

 private:
  FILE* out_file_ = nullptr;
  absl::flat_hash_set<std::string> hashes_;
};

/// \brief An interface for processing elements discovered as part of a
/// compilation unit.
///
/// Before calling a member on a `GraphObserver` that accepts a `SourceLocation`
/// as an argument, the `GraphObserver`'s `SourceManager` must be set.
class GraphObserver {
 public:
  /// \brief Provides additional information about an object that may be
  /// used to determine responsibility for processing it.
  ///
  /// ClaimTokens are owned by the `GraphObserver` and are destroyed
  /// with the `GraphObserver`. The information they represent is specific
  /// to each `GraphObserver` implementation and may include the claimable
  /// object's representative source path, whether a claim has been successfully
  /// made (thus making the token active), and so on.
  class ClaimToken {
   public:
    virtual ~ClaimToken(){};
    /// \brief Returns a string representation of `Identity` stamped with this
    /// token.
    virtual std::string StampIdentity(const std::string& Identity) const = 0;
    /// \brief Returns a value unique to each implementation of `ClaimToken`.
    virtual uintptr_t GetClass() const = 0;
    /// \brief Checks for equality.
    ///
    /// `ClaimTokens` are only equal if they have the same value for `GetClass`.
    virtual bool operator==(const ClaimToken& RHS) const = 0;
    virtual bool operator!=(const ClaimToken& RHS) const = 0;
  };

  /// \brief Takes care of balancing calls to `Delimit` and `Undelimit`.
  class Delimiter {
   public:
    Delimiter(GraphObserver& Self) : S(Self) { S.Delimit(); }
    ~Delimiter() { S.Undelimit(); }

   private:
    GraphObserver& S;
  };

  /// \brief Uses a one-way function to encode `InString`. Guaranteed to return
  /// only websafe base64 characters.
  std::string ForceEncodeString(absl::string_view InString) const;

  /// \brief Uses a one-way function to compress `InString` if it's longer than
  /// the result of using that function would be.
  std::string CompressString(absl::string_view InString) const;

  /// \brief Uses a one-way function to compress the anchor signature
  /// `InSignature` if it's longer than the result of using that function would
  /// be.
  std::string CompressAnchorSignature(absl::string_view InSignature) const;

  /// \brief Push another group onto the group stack, assigning
  /// any observations that follow to it.
  virtual void Delimit() {}
  /// \brief Pop the last group from the group stack.
  virtual void Undelimit() {}

  /// \brief The identifier for an object in the graph being observed.
  ///
  /// A node is identified uniquely in the graph by its `Token`, which
  /// provides evidence of its provenance (and may be used to determine whether
  /// the node should be analyzed), and its `Identity`, a string of bytes
  /// determined by the `IndexerASTHooks` and `GraphObserver` override.
  class NodeId {
   public:
    NodeId() {}
    NodeId(const NodeId& C) { *this = C; }
    NodeId& operator=(const NodeId* C) {
      Token = C->Token;
      Identity = C->Identity;
      return *this;
    }
    static NodeId CreateUncompressed(const ClaimToken* Token,
                                     const std::string& Identity) {
      NodeId NewId(Token, "");
      NewId.Identity = Identity;
      return NewId;
    }
    /// \brief Returns a string representation of this `NodeId`.
    std::string ToString() const { return Identity; }
    /// \brief Returns a string reference representation of this `NodeId`'s
    /// Identity.
    /// This `NodeId` must outlive the `StringRef`.
    llvm::StringRef IdentityRef() const {
      return llvm::StringRef(Identity.data(), Identity.size());
    }
    /// \brief Returns a string representation of this `NodeId`
    /// annotated by its claim token.
    std::string ToClaimedString() const {
      return Token->StampIdentity(Identity);
    }
    bool operator==(const NodeId& RHS) const {
      return *Token == *RHS.Token && Identity == RHS.Identity;
    }
    bool operator!=(const NodeId& RHS) const {
      return *Token != *RHS.Token || Identity != RHS.Identity;
    }
    const std::string& getRawIdentity() const { return Identity; }
    const ClaimToken* getToken() const { return Token; }

   private:
    friend class GraphObserver;
    explicit NodeId(const ClaimToken* Token, const std::string& Identity)
        : Token(Token), Identity(Identity) {}

    const ClaimToken* Token = nullptr;
    std::string Identity;
  };

  NodeId MakeNodeId(const ClaimToken* Token,
                    const std::string& Identity) const {
    return NodeId(Token, CompressString(Identity));
  }

  /// \brief A range of source text, potentially associated with a node.
  ///
  /// The `GraphObserver` interface uses `clang::SourceRange` instances to
  /// locate source text. It also supports associating a source range with
  /// a semantic context. This is used to distinguish generated code from the
  /// code that generates it. For instance, ranges specific to an implicit
  /// specialization of some type may have physical ranges that overlap with
  /// the primary template's, but they will be distinguished from the primary
  /// template's ranges by being associated with the context of the
  /// specialization.
  struct Range {
    enum class RangeKind {
      /// This range relates to a run of bytes in a source file.
      Physical,
      /// This range refers to source text that was synthesized by the compiler,
      /// but which is not associated with any written code. For example,
      /// the ranges associated with implicitly defined constructors are
      /// Implicit. Ranges that are associated with implicit instantiations
      /// of templates are Wraiths.
      Implicit,
      /// This range is related to some bytes, but also lives in an
      /// imaginary context; for example, a declaration of a member variable
      /// inside an implicit template instantiation has its source range
      /// set to the declaration's range in the template being instantiated
      /// and its context set to the `NodeId` of the implicit specialization.
      Wraith
    };
    /// \brief Constructs a physical `Range` for the given `clang::SourceRange`.
    Range(const clang::SourceRange& R, const ClaimToken* T)
        : Kind(RangeKind::Physical), PhysicalRange(R), Context(T, "") {
      CHECK(R.getBegin().isValid());
    }
    /// \brief Constructs a `Range` with some physical location, but specific to
    /// the context of some semantic node.
    Range(const clang::SourceRange& R, const NodeId& C)
        : Kind(RangeKind::Wraith), PhysicalRange(R), Context(C) {
      CHECK(R.getBegin().isValid());
    }
    /// \brief Constructs a new `Range` in the context of an existing
    /// `Range`, but with a different physical location.
    Range(const Range& R, const clang::SourceRange& NR)
        : Kind(R.Kind), PhysicalRange(NR), Context(R.Context) {
      CHECK(NR.getBegin().isValid());
    }
    /// \brief Constructs a new implicit `Range` with a source range hint,
    /// if that source location is valid, not a macro location, and is
    /// in a physical file.
    static Range Implicit(const NodeId& C, const clang::SourceRange& NR) {
      Range new_range(C);
      if (NR.getBegin().isValid() && NR.getBegin().isFileID()) {
        new_range.PhysicalRange = NR;
        new_range.PhysicalRange.setEnd(new_range.PhysicalRange.getBegin());
      }
      return new_range;
    }
    /// \brief Constructs a new `Implicit` `Range` keyed on a semantic object.
    explicit Range(const NodeId& C) : Kind(RangeKind::Implicit), Context(C) {}
    RangeKind Kind;
    clang::SourceRange PhysicalRange;
    NodeId Context;
  };

  struct NameId {
    /// \brief C++ distinguishes between several equivalence classes of names,
    /// a selection of which we consider important to represent.
    enum class NameEqClass {
      None,   ///< This name is not a member of a significant class.
      Union,  ///< This name names a union record.
      Class,  ///< This name names a non-union class or struct.
      Macro   ///< This name names a macro.
      // TODO(zarko): Enums should be part of their own NameEqClass.
      // We should consider whether representation type information should
      // be part of enum (lookup) names--or whether this information must
      // be queried separately by clients and postprocessors.
    };
    /// \brief The abstract path to this name. For most `NameEqClass`es, this
    /// is the path from the translation unit root.
    std::string Path;
    /// \brief The `NameClass` of this name.
    NameEqClass EqClass;
    /// \brief Whether this name is known to be hidden.
    ///
    /// Most useful names can be uttered from the global namespace.
    /// Some names, like the names of local variables, cannot. When we know
    /// a name is unutterable, this flag is set.
    bool Hidden = false;
    /// \brief Returns a string representation of this `NameId`.
    std::string ToString() const;
  };

  /// \brief Determines whether an edge an opt out of claiming.
  ///
  /// If claiming based on the preprocessor is not enough to determine whether
  /// an edge adds unique content to the graph, that edge may be marked as
  /// Unclaimable.
  enum class Claimability {
    Claimable,   ///< This edge may be dropped by claiming.
    Unclaimable  ///< This edge must always be emitted.
  };

  /// \brief Classifies a use site.
  enum class UseKind {
    /// No specific determination. Similar to a read.
    kUnknown,
    /// This use site is a write.
    kWrite,
    /// This use site is both a read and a write.
    kReadWrite,
    /// This use site takes an alias.
    kTakeAlias,
  };

  GraphObserver() {}

  /// \brief Sets the `SourceManager` that this `GraphObserver` should use.
  ///
  /// Since the `SourceManager` may not exist at the time the
  /// `GraphObserver` is created, it must be set after construction.
  ///
  /// \param SM the context for all `SourceLocation` instances.
  virtual void setSourceManager(clang::SourceManager* SM) {
    SourceManager = SM;
  }

  /// \brief Loads and applies the metadata file `FE` to the given FileId.
  /// \param SearchString if non-empty, is used to locate a C-style comment
  /// inside `FE` that contains metadata.
  /// \param TargetFile the file to which the metadata is being applied.
  virtual void applyMetadataFile(clang::FileID ID, const clang::FileEntry* FE,
                                 const std::string& SearchString,
                                 const clang::FileEntry* TargetFile) {}

  /// \param LO the language options in use.
  virtual void setLangOptions(const clang::LangOptions* LO) {
    LangOptions = LO;
  }

  /// \param PP The `Preprocessor` to use.
  virtual void setPreprocessor(clang::Preprocessor* PP) { Preprocessor = PP; }

  /// \brief Returns a claim token that provides no additional information.
  virtual const ClaimToken* getDefaultClaimToken() const = 0;

  /// \brief Returns a claim token that identifies holders as VNames.
  virtual const ClaimToken* getVNameClaimToken() const = 0;

  /// \brief Returns a claim token for namespaces declared at `Loc`.
  /// \param Loc The declaration site of the namespace.
  virtual const ClaimToken* getNamespaceClaimToken(
      clang::SourceLocation Loc) const {
    return getDefaultClaimToken();
  }

  /// \brief Returns a claim token for anonymous namespaces declared at `Loc`.
  /// \param Loc The declaration site of the anonymous namespace.
  virtual const ClaimToken* getAnonymousNamespaceClaimToken(
      clang::SourceLocation Loc) const {
    return getDefaultClaimToken();
  }

  /// \brief Returns the `NodeId` for the builtin type or type constructor named
  /// by `Spelling`.
  ///
  /// We invent names for type constructors, which are ordinarly spelled out
  /// as operators in C++. These names are "ptr" for *, "rref" for &&,
  /// "lref" for &, "const", "volatile", and "restrict".
  ///
  /// \param Spelling the spelling of the builtin type (from
  /// `clang::BuiltinType::getName(this->getLangOptions())` or the builtin
  /// type constructor.
  /// \return The `NodeId` for `Spelling`.
  virtual NodeId getNodeIdForBuiltinType(llvm::StringRef Spelling) const = 0;

  /// \brief Returns the ID for a type node aliasing another type node.
  /// \param AliasName a `NameId` for the alias name.
  /// \param AliasedType a `NodeId` corresponding to the aliased type.
  /// \return the `NodeId` for the type node corresponding to the alias.
  virtual NodeId nodeIdForTypeAliasNode(const NameId& AliasName,
                                        const NodeId& AliasedType) const = 0;

  /// \brief Records a type alias node (eg, from a `typedef` or
  /// `using Alias = ty` instance).
  /// \param AliasName a `NameId` for the alias name.
  /// \param AliasedType a `NodeId` corresponding to the aliased type.
  /// \param RootAliasedType the non-alias at the root of the alias chain; may
  /// be AliasedType
  /// \param MarkedSource marked source for the alias.
  /// \return the `NodeId` for the type alias node this definition defines.
  NodeId recordTypeAliasNode(const NameId& AliasName, const NodeId& AliasedType,
                             const std::optional<NodeId>& RootAliasedType,
                             const std::optional<MarkedSource>& MarkedSource) {
    return recordTypeAliasNode(nodeIdForTypeAliasNode(AliasName, AliasedType),
                               AliasedType, RootAliasedType, MarkedSource);
  }

  /// \brief Records a type alias node (eg, from a `typedef` or
  /// `using Alias = ty` instance).
  /// \param AliasName a `NodeId` for the alias.
  /// \param AliasedType a `NodeId` corresponding to the aliased type.
  /// \param RootAliasedType the non-alias at the root of the alias chain; may
  /// be AliasedType
  /// \param MarkedSource marked source for the alias.
  /// \return the `NodeId` for the type alias node this definition defines.
  virtual NodeId recordTypeAliasNode(
      const NodeId& AliasId, const NodeId& AliasedType,
      const std::optional<NodeId>& RootAliasedType,
      const std::optional<MarkedSource>& MarkedSource) = 0;

  /// \brief Returns the ID for a nominal type node (such as a struct,
  /// typedef or enum).
  /// \param TypeName a `NameId` corresponding to a nominal type.
  /// \return the `NodeId` for the type node corresponding to `TypeName`.
  virtual NodeId nodeIdForNominalTypeNode(const NameId& TypeName) const = 0;

  /// \brief Records a type node for some nominal type (such as a struct,
  /// typedef or enum), returning its ID.
  /// \param TypeName a `NameId` corresponding to a nominal type.
  /// \param MarkedSource marked source for the type node.
  /// \param Parent if non-null, the parent node of this nominal type.
  /// \return the `NodeId` for the type node corresponding to `TypeName`.
  NodeId recordNominalTypeNode(const NameId& TypeName,
                               const std::optional<MarkedSource>& MarkedSource,
                               const std::optional<NodeId>& Parent) {
    return recordNominalTypeNode(nodeIdForNominalTypeNode(TypeName),
                                 MarkedSource, Parent);
  }

  /// \brief Records a type node for some nominal type (such as a struct,
  /// typedef or enum), returning its ID.
  /// \param TypeNode a `NodeId` corresponding to a nominal type.
  /// \param MarkedSource marked source for the type node.
  /// \param Parent if non-null, the parent node of this nominal type.
  /// \return the `NodeId` for the type node.
  virtual NodeId recordNominalTypeNode(
      const NodeId& TypeNode, const std::optional<MarkedSource>& MarkedSource,
      const std::optional<NodeId>& Parent) = 0;

  /// \brief Returns a type application node ID.
  /// \note This is the elimination form for the `abs` node.
  /// \param TyconId The `NodeId` of the appropriate type constructor or
  /// abstraction.
  /// \param Params The `NodeId`s of the types to apply to the constructor or
  /// abstraction.
  /// \return The application's result's ID.
  virtual NodeId nodeIdForTappNode(const NodeId& TyconId,
                                   absl::Span<const NodeId> Params) const = 0;

  /// \brief Records a type application node, returning its ID.
  /// \note This is the elimination form for the `abs` node.
  /// \param TappId The `NodeId` of the tapp to record.
  /// \param TyconId The `NodeId` of the appropriate type constructor or
  /// abstraction.
  /// \param Params The `NodeId`s of the types to apply to the constructor or
  /// abstraction.
  /// \param FirstDefaultParam The first type parameter to consider
  /// assigned its default value (or Params.size() otherwise).
  /// \return The application's result's ID.
  virtual NodeId recordTappNode(const NodeId& TappId, const NodeId& TyconId,
                                absl::Span<const NodeId> Params,
                                unsigned FirstDefaultParam) = 0;

  /// \brief Records a type application node, returning its ID.
  /// \note This is the elimination form for the `abs` node.
  /// \param TyconId The `NodeId` of the appropriate type constructor or
  /// abstraction.
  /// \param Params The `NodeId`s of the types to apply to the constructor or
  /// abstraction.
  /// \param FirstDefaultParam The first type parameter to consider
  /// assigned its default value (or Params.size() otherwise).
  /// \return The application's result's ID.
  NodeId recordTappNode(const NodeId& TyconId, absl::Span<const NodeId> Params,
                        unsigned FirstDefaultParam) {
    return recordTappNode(nodeIdForTappNode(TyconId, Params), TyconId, Params,
                          FirstDefaultParam);
  }

  /// \brief Records a type application node, returning its ID.
  /// \note This is the elimination form for the `abs` node.
  /// \param TyconId The `NodeId` of the appropriate type constructor or
  /// abstraction.
  /// \param Params The `NodeId`s of the types to apply to the constructor or
  /// abstraction.
  /// Uses Params.size() for FirstDefaultParam.
  /// \return The application's result's ID.
  NodeId recordTappNode(const NodeId& TyconId,
                        absl::Span<const NodeId> Params) {
    return recordTappNode(TyconId, Params, Params.size());
  }

  /// \brief Returns the ID for a tsigma type node (e.g. a C++ parameter pack
  /// substitution).
  /// \param Params The `NodeId`s of the types to include.
  /// \return The result's ID.
  virtual NodeId nodeIdForTsigmaNode(absl::Span<const NodeId> Params) const = 0;

  /// \brief Records a sigma node, returning its ID.
  /// \param TsigmaId The `NodeId` to record.
  /// \param Params The `NodeId`s of the types to include.
  /// \return The result's ID.
  virtual NodeId recordTsigmaNode(const NodeId& TsigmaId,
                                  absl::Span<const NodeId> Params) = 0;

  /// \brief Records a sigma node, returning its ID.
  /// \param Params The `NodeId`s of the types to include.
  /// \return The result's ID.
  NodeId recordTsigmaNode(absl::Span<const NodeId> Params) {
    return recordTsigmaNode(nodeIdForTsigmaNode(Params), Params);
  }

  enum class RecordKind { Struct, Class, Union, Category };

  /// \brief Describes how autological a given declaration is.
  enum class Completeness {
    /// This declaration is a definition (`class C {};`) and hence is
    /// necessarily also complete.
    Definition,
    /// This declaration is complete (in the sense of [basic.types]),
    /// but it may not be a definition (`enum class E : short;`) [dcl.enum]
    Complete,
    /// This declaration is incomplete (`class C;`)
    Incomplete
  };

  /// \brief Selects from among different flavors of variable.
  enum class VariableSubkind {
    /// This variable has no specific subkind.
    None,
    /// This is a field.
    Field
  };

  enum class FunctionSubkind {
    /// This function has no specific subkind.
    None,
    /// This is a constructor.
    Constructor,
    /// This is a destructor.
    Destructor,
    /// This is a synthesized initializer.
    Initializer
  };

  enum class Variance { Covariant, Contravariant, Invariant };

  /// \brief Records a node representing an interface type (such as a protocol
  /// in Objective-C).
  /// \param Node The NodeId of the record.
  /// \param MarkedSource marked source for this interface.
  virtual void recordInterfaceNode(
      const NodeId& Node, const std::optional<MarkedSource>& MarkedSource) {}

  /// \brief Records a node representing a record type (such as a class or
  /// struct).
  /// \param Node The NodeId of the record.
  /// \param Kind Whether this record is a struct, class, or union.
  /// \param RecordCompleteness Whether the record is complete.
  /// \param MarkedSource marked source for this record.
  virtual void recordRecordNode(
      const NodeId& Node, RecordKind Kind, Completeness RecordCompleteness,
      const std::optional<MarkedSource>& MarkedSource) {}

  /// \brief Records a node representing a function.
  /// \param Node The NodeId of the function.
  /// \param FunctionCompleteness Whether the function is complete.
  /// \param Subkind The subkind of the function.
  /// \param MarkedSource marked source for this function.
  virtual void recordFunctionNode(
      const NodeId& Node, Completeness FunctionCompleteness,
      FunctionSubkind Subkind,
      const std::optional<MarkedSource>& MarkedSource) {}

  /// \brief Assigns a USR to node.
  /// \param Node The target node.
  /// \param Usr The raw USR.
  /// \param ByteSize Number of significant bytes to use.
  virtual void assignUsr(const NodeId& Node, llvm::StringRef Usr,
                         int ByteSize) {}

  /// \brief Describes whether an enum is scoped (`enum class`).
  enum class EnumKind {
    Scoped,   ///< This enum is scoped (an `enum class`).
    Unscoped  ///< This enum is unscoped (a plain `enum`).
  };

  /// \brief Explicitly record marked source for some `Node`.
  virtual void recordMarkedSource(
      const NodeId& Node, const std::optional<MarkedSource>& MarkedSource) {}

  /// \brief Records a node representing a variable in a dependent type
  /// abstraction.
  /// \param Node The `NodeId` of the variable.
  /// \param MarkedSource marked source for this variable.
  virtual void recordTVarNode(const NodeId& Node,
                              const std::optional<MarkedSource>& MarkedSource) {
  }

  /// \brief Records a node representing a deferred lookup.
  /// \param Node The `NodeId` of the lookup.
  /// \param Name The `Name` for which resolution has been deferred
  virtual void recordLookupNode(const NodeId& Node, llvm::StringRef Name) {}

  /// \brief Records a parameter relationship.
  /// \param `ParamOfNode` The node this `ParamNode` is the parameter of.
  /// \param `Ordinal` The ordinal for the parameter (0 is the first).
  /// \param `ParamNode` The `NodeId` for the parameter.
  virtual void recordParamEdge(const NodeId& ParamOfNode, uint32_t Ordinal,
                               const NodeId& ParamNode) {}

  /// \brief Records a type parameter relationship.
  /// \param `ParamOfNode` The node this `ParamNode` is the type parameter of.
  /// \param `Ordinal` The ordinal for the parameter (0 is the first).
  /// \param `ParamNode` The `NodeId` for the parameter.
  virtual void recordTParamEdge(const NodeId& ParamOfNode, uint32_t Ordinal,
                                const NodeId& ParamNode) {}

  /// \brief Records a node representing an enumerated type.
  /// \param Compl Whether the enum is complete.
  /// \param EnumKind Whether the enum is scoped.
  virtual void recordEnumNode(const NodeId& Node, Completeness Compl,
                              EnumKind Kind) {}

  /// \brief Records a node representing a constant with a value representable
  /// with an integer.
  ///
  /// Note that the type of the language-level object this node represents may
  /// not be integral. For example, `recordIntegerConstantNode` is used to
  /// record information about the implicitly or explicitly assigned values
  /// for enumerators.
  virtual void recordIntegerConstantNode(const NodeId& Node,
                                         const llvm::APSInt& Value) {}
  // TODO(zarko): Handle other values. How should we represent class
  // constants?

  /// \brief Records that a variable (either local or global) has been
  /// declared.
  /// \param DeclNode The identifier for this particular element.
  /// \param Compl The completeness of this variable declaration.
  /// \param Subkind Which kind of variable declaration this is.
  /// \param MarkedSource marked source for this variable.
  // TODO(zarko): We should make note of the storage-class-specifier (dcl.stc)
  // of the variable, which is a property the variable itself and not of its
  // type.
  virtual void recordVariableNode(
      const NodeId& DeclNode, Completeness Compl, VariableSubkind Subkind,
      const std::optional<MarkedSource>& MarkedSource) {}

  /// \brief Records that a namespace has been declared.
  /// \param DeclNode The identifier for this particular element.
  /// \param MarkedSource marked source for this namespace.
  virtual void recordNamespaceNode(
      const NodeId& DeclNode, const std::optional<MarkedSource>& MarkedSource) {
  }

  // TODO(zarko): recordExpandedTypeEdge -- records that a type was seen
  // to have some canonical type during a compilation. (This is a 'canonical'
  // type for a given compilation, but may differ between compilations.)

  /// \brief Records that a particular `Range` contains the declaration
  /// of the node called `DeclId` (with possible full definition `DefnId`).
  ///
  /// The provided `Range` should cover the full syntactic definition of the
  /// identified node.
  virtual void recordFullDefinitionRange(
      const Range& SourceRange, const NodeId& DeclId,
      const std::optional<NodeId>& DefnId = std::nullopt) {}

  /// \brief Should an anchor be stamped
  enum class Stamping { Unstamped, Stamped };

  /// \brief Records that a particular `Range` contains the declaration
  /// of the node called `DeclId` (with possible full definition `DefnId`).
  ///
  /// Generally the `BindingRange` provided will be small and limited only to
  /// the part of the declaration that binds a name. For example, in `class C`,
  /// we would `recordDefinitionBindingRange` on the range for `C`.
  virtual void recordDefinitionBindingRange(
      const Range& BindingRange, const NodeId& DeclId,
      const std::optional<NodeId>& DefnId = std::nullopt,
      Stamping stamping = Stamping::Stamped) {}

  /// \brief Records that a particular `Range` contains the declaration
  /// of the node called `DeclId` (with possible full definition `DefnId`).
  ///
  /// Generally the `BindingRange` provided will be small and limited only to
  /// the part of the declaration that binds a name. For example, in `class C`,
  /// we would `recordDefinitionRangeWithBinding` on the range for `C`.
  /// The `SourceRange` should cover the full syntactic definition of the
  /// identified node.
  virtual void recordDefinitionRangeWithBinding(
      const Range& SourceRange, const Range& BindingRange, const NodeId& DeclId,
      const std::optional<NodeId>& DefnId = std::nullopt) {}

  /// \brief Records that a particular string contains documentation for
  /// the node called `DocId`, possibly containing inner links to other nodes.
  virtual void recordDocumentationText(const NodeId& DocId,
                                       const std::string& DocText,
                                       const std::vector<NodeId>& DocLinks) {}

  /// \brief Records that a particular `Range` contains some documentation
  /// for the node called `DocId`.
  virtual void recordDocumentationRange(const Range& SourceRange,
                                        const NodeId& DocId) {}

  /// \brief Determines whether a feature is explicit or implicit.
  enum class Implicit {
    /// This feature is explicit (e.g., it's written down in source).
    No,
    /// This feature is implicit (e.g., it's the result of macro expansion
    /// or template instantiation).
    Yes
  };

  /// \brief Determines whether a call will perform dynamic dispatch.
  enum class CallDispatch {
    /// This call may perform dynamic dispatch.
    kDefault,
    /// This call will not perform dynamic dispatch.
    kDirect
  };

  /// \brief Records a use site for a decl inside some documentation.
  virtual void recordDeclUseLocationInDocumentation(const Range& SourceRange,
                                                    const NodeId& DeclId) {}

  /// \brief Describes how much of a guess an edge is.
  enum class Confidence {
    /// This relationship definitely exists.
    NonSpeculative,
    /// There is not enough information to determine whether this relationship
    /// actually exists.
    Speculative
  };

  /// \brief Records that a particular `CompletingNode` completing the node
  /// named `DefnId`.
  /// \param DefnId The `NodeId` for the node being completed.
  /// \param CompletingNode The node completing DefnId. This refers to, for
  /// example, the function definition that completes a declaration. In the case
  /// where there are multiple possible nodes, like when the function is
  /// actually a function template, pass the ID for the outer (abs) node.
  virtual void recordCompletion(const NodeId& DefnId,
                                const NodeId& CompletingNode) {}

  /// \brief Records the type of a node as an edge in the graph.
  /// \param TermNodeId The identifier for the node to be given a type.
  /// \param TypeNodeId The identifier for the node representing the type.
  virtual void recordTypeEdge(const NodeId& TermNodeId,
                              const NodeId& TypeNodeId) {}

  /// \brief Records that `Influencer` influences `Influenced`.
  virtual void recordInfluences(const NodeId& Influencer,
                                const NodeId& Influenced) {}

  /// \brief Records an upper bound for the type of a node as an edge in the
  /// graph.
  /// \param TypeNodeId The identifier for the node to given a bounded type.
  /// \param TypeNodeId The identifier for the node representing the type that
  /// is the bound.
  virtual void recordUpperBoundEdge(const NodeId& TypeNodeId,
                                    const NodeId& TypeBoundNodeId) {}

  /// \brief Record the variance for this Type Node. This is useful for
  /// Objective-C where the user can explicitly set the variance for a type
  /// parameter.
  ///
  /// Objective C type param decls may declare their variance, which affects the
  /// subtyping of the parent type. See the test cases for examples and text
  /// explaining the typing relationship.
  ///
  /// In short, given A<__covaraint T : U*>, the type parameter bound (T : U*)
  /// describes how the type parameter can be assigned. The co/contra qualifier
  /// describes the type relationship between A<X*> and A<Y*> if X and Y are
  /// both subtypes of U.
  virtual void recordVariance(const NodeId& TypeNodeId, const Variance V) {}

  /// \brief Records that some term specializes some abstraction.
  /// \param TermNodeId The identifier for the node specializing the
  /// abstraction.
  /// \param TypeNodeId The identifier for the node representing the specialized
  /// abstraction.
  /// \param Conf Whether we're sure that this relationship exists.
  virtual void recordSpecEdge(const NodeId& TermNodeId, const NodeId& AbsNodeId,
                              Confidence Conf) {}

  /// \brief Records that some term instantiates some abstraction.
  /// \param TermNodeId The identifier for the node instantiating the type.
  /// \param TypeNodeId The identifier for the node representing the
  /// instantiated abstraction.
  /// \param Conf Whether we're sure that this relationship exists.
  virtual void recordInstEdge(const NodeId& TermNodeId, const NodeId& AbsNodeId,
                              Confidence Conf) {}

  /// \brief Records that some overrider overrides a base object.
  /// \param Overrider the object doing the overriding
  /// \param BaseObject the object being overridden
  virtual void recordOverridesEdge(const NodeId& Overrider,
                                   const NodeId& BaseObject) {}

  /// \brief Records that some overrider overrides a root base object.
  /// \param Overrider the object doing the overriding
  /// \param RootBaseObject the root of the override chain; may be BaseObject
  virtual void recordOverridesRootEdge(const NodeId& Overrider,
                                       const NodeId& RootBaseObject) {}

  /// \brief Records that a node is called at a particular location.
  /// \param CallLoc The `Range` responsible for making the call.
  /// \param CallerId The scope to be held responsible for making the call;
  /// for example, a function.
  /// \param CalleeId The node being called.
  /// \param I Whether this call is implicit.
  /// \param D Whether this call is direct.
  virtual void recordCallEdge(const Range& SourceRange, const NodeId& CallerId,
                              const NodeId& CalleeId, Implicit I,
                              CallDispatch D = CallDispatch::kDefault) {}

  /// \brief Creates a node to represent a file-level initialization routine
  /// that can be blamed for a call at `CallSite`.
  /// \param CallSite The call site that needs a routine to blame.
  /// \return The `NodeId` for the routine to blame.
  virtual std::optional<NodeId> recordFileInitializer(const Range& CallSite) {
    return std::nullopt;
  }

  /// \brief Records a child-to-parent relationship as an edge in the graph.
  /// \param ChildNodeId The identifier for the child node.
  /// \param ParentNodeId The identifier for the parent node.
  virtual void recordChildOfEdge(const NodeId& ChildNodeId,
                                 const NodeId& ParentNodeId) {}

  /// \brief Records that a record adds functionality to another record. In the
  /// case of Objective-C this occurs in a category where additional methods
  /// are added to a preexisting class.
  /// \param InheritingNodeId The record type providing additional
  /// functionality. In Objective-C, this would be the category.
  /// \param InheritedTypeId The record type being extended (the base class).
  virtual void recordCategoryExtendsEdge(const NodeId& InheritingNodeId,
                                         const NodeId& InheritedTypeId) {}

  /// \brief Records that a record directly inherits from another record.
  /// \param InheritingNodeId The inheriting record type (the derived class).
  /// \param InheritedTypeId The record type being inherited (the base class).
  /// \param IsVirtual True if the inheritance is virtual.
  /// \param AS The access specifier for this inheritance.
  virtual void recordExtendsEdge(const NodeId& InheritingNodeId,
                                 const NodeId& InheritedTypeId, bool IsVirtual,
                                 clang::AccessSpecifier AS) {}

  /// \brief Records a node with a provided kind.
  /// \param Id The ID of the node to record.
  /// \param NodeKind The kind of the node ("google/protobuf")
  /// \param Compl Whether this node is complete.
  virtual void recordUserDefinedNode(const NodeId& Id, llvm::StringRef NodeKind,
                                     const std::optional<Completeness> Compl) {}

  /// \brief Records a use site for some decl.
  virtual void recordDeclUseLocation(const Range& SourceRange,
                                     const NodeId& DeclId, Claimability Cl,
                                     Implicit I) {}

  /// \brief Blames a given source range on the given context.
  virtual void recordBlameLocation(const Range& SourceRange,
                                   const NodeId& BlameId, Claimability Cl,
                                   Implicit I) {}

  /// \brief Records a use site for some decl with additional semantic
  /// information.
  virtual void recordSemanticDeclUseLocation(const Range& SourceRange,
                                             const NodeId& DeclId, UseKind K,
                                             Claimability Cl, Implicit I) {}

  /// \brief Records an init site for some decl.
  virtual void recordInitLocation(const Range& SourceRange,
                                  const NodeId& DeclId, Claimability Cl,
                                  Implicit I) {}

  /// \brief Records that a type was spelled out at a particular location.
  /// \param SourceRange The source range covering the type spelling.
  /// \param TypeNode The identifier for the type being spelled out.
  /// \param Cr Whether this information can be dropped by claiming.
  virtual void recordTypeSpellingLocation(const Range& SourceRange,
                                          const NodeId& TypeId, Claimability Cl,
                                          Implicit I) {}

  /// \brief Records that a type was spelled out at a particular location, while
  /// referencing a different entity.
  /// \param SourceRange The source range covering the type spelling.
  /// \param TypeNode The identifier for the type being spelled out.
  /// \param Cr Whether this information can be dropped by claiming.
  virtual void recordTypeIdSpellingLocation(const Range& SourceRange,
                                            const NodeId& TypeId,
                                            Claimability Cl, Implicit I) {}

  /// \brief Records that a macro was defined.
  virtual void recordMacroNode(const NodeId& MacroNode) {}

  /// \brief Records that a macro was expanded at some location.
  ///
  /// This function is called when a macro with some (potentially empty)
  /// definition is substituted for that definition. This is distinct from
  /// the `recordBoundQueryRange` and `recordUnboundQueryRange` events, which
  /// happen when a macro's binding state (whether it has or has no definition
  /// at all, respectively) is tested. Testing a macro for definedness does
  /// not expand it.
  ///
  /// \param SourceRange The `Range` covering the text causing the expansion.
  /// \param MacroId The `NodeId` of the macro being expanded.
  /// \sa recordBoundQueryRange, recordUnboundQueryRange
  /// \sa recordIndirectlyExpandsRange
  virtual void recordExpandsRange(const Range& SourceRange,
                                  const NodeId& MacroId) {}

  /// \brief Records that a macro was expanded indirectly at some location.
  ///
  /// This function is called when a macro with some (potentially empty)
  /// definition is substituted for that definition because an initial
  /// macro substitution was made at the provided source location.
  ///
  /// \param SourceRange The `Range` covering the original text causing the
  /// (first) expansion.
  /// \param MacroId The `NodeId` of the macro being expanded.
  /// \sa recordBoundQueryRange, recordUnboundQueryRange, recordExpandsRange
  virtual void recordIndirectlyExpandsRange(const Range& SourceRange,
                                            const NodeId& MacroId) {}

  /// \brief Records that a macro was undefined at some location.
  /// \param SourceRange The `Range` covering the text causing the undefinition.
  /// \param MacroId The `NodeId` of the macro being undefined.
  virtual void recordUndefinesRange(const Range& SourceRange,
                                    const NodeId& MacroId) {}

  /// \brief Records that a defined macro was queried at some location.
  ///
  /// Testing a macro for definedness does not expand it.
  ///
  /// \param SourceRange The `Range` covering the text causing the query.
  /// \param MacroId The `NodeId` of the macro being queried.
  virtual void recordBoundQueryRange(const Range& SourceRange,
                                     const NodeId& MacroId) {}

  /// \brief Records that another resource was included at some location.
  /// \param SourceRange The `Range` covering the text causing the inclusion.
  /// \param File The resource being included.
  virtual void recordIncludesRange(const Range& SourceRange,
                                   const clang::FileEntry* File) {}

  /// \brief Records that the specified variable is static.
  /// \param VarNodeId The `NodeId` of the static variable.
  virtual void recordStaticVariable(const NodeId& VarNodeId) {}

  /// \brief Records the visibility of the specified field.
  /// \param FieldNodeId The `NodeId` of the field.
  virtual void recordVisibility(const NodeId& FieldNodeId,
                                clang::AccessSpecifier access) {}

  /// \brief Records that the specified node is deprecated.
  /// \param NodeId The `NodeId` of the deprecated node.
  /// \param Advice A user-readable message about the deprecation (or empty).
  virtual void recordDeprecated(const NodeId& NodeId, llvm::StringRef Advice) {}

  /// \brief Records a diagnostic at the given source range
  /// \param Range The source range
  /// \param Signature The signature to use for the diagnostic VName
  /// \param Message The diagnostic message
  virtual void recordDiagnostic(const Range& Range, llvm::StringRef Signature,
                                llvm::StringRef Message) {}

  /// \brief Called when a new input file is entered.
  ///
  /// The file entered in the first `pushFile` is the compilation unit being
  /// indexed.
  ///
  /// \param BlameLocation If valid, the `SourceLocation` that caused the file
  /// to be pushed, e.g., an include directive.
  /// \param Loc A `SourceLocation` in the file being entered.
  /// \sa popFile
  virtual void pushFile(clang::SourceLocation BlameLocation,
                        clang::SourceLocation Location) {}

  /// \brief Called when the previous input file to be entered is left.
  /// \sa pushFile
  virtual void popFile() {}

  /// \brief Returns true if the given source location is part of the
  /// main source file (e.g., it was written in the main source file or
  /// a textual include, but not a header include or a textual include from
  /// a header include).
  /// \pre Preprocessing is complete.
  virtual bool isMainSourceFileRelatedLocation(
      clang::SourceLocation Location) const {
    // Conservatively return true.
    return true;
  }

  /// \brief Append a string representation of the identifier of the main
  /// source file to the given stream.
  /// \pre Preprocessing is complete.
  virtual void AppendMainSourceFileIdentifierToStream(
      llvm::raw_ostream& Ostream) const {}

  /// \brief Checks whether this `GraphObserver` should emit data for some
  /// `NodeId` and its descendants.
  ///
  /// \param NodeId The node to claim.
  ///
  /// It is always safe to return `true` here (as this will result in
  /// redundant output instead of dropped output).
  virtual bool claimNode(const NodeId& NodeId) { return true; }

  /// \brief Checks whether this `GraphObserver` should emit data for
  /// an implicitly defined node which may not have a claim token.
  ///
  /// It is always safe to return `true` here. If this function returns
  /// `true`, the caller should follow up with a call to `finishImplicitNode`.
  virtual bool claimImplicitNode(const std::string& Identifier) { return true; }

  /// \brief Notifies the `GraphObserver` that data that was emitted because
  /// a call to `claimImplicitNode` returned true.
  virtual void finishImplicitNode(const std::string& Identifier) {}

  /// \brief Claim a batch of identifying tokens for an anonymous claimant.
  /// \param tokens A vector of pairs of `(token, claimed)`. `claimed` should
  /// initially be set to `true`.
  /// \return true if, after claiming, any of the `claimed` values is set to
  /// `true`.
  ///
  /// Calls to `claimBatch` are not idempotent; claiming the same token more
  /// than once may fail even if the first claim succeeds. Implementations
  /// should ensure that failure is permanent.
  virtual bool claimBatch(std::vector<std::pair<std::string, bool>>* pairs) {
    bool claimed = false;
    for (auto& pair : *pairs) {
      if (claimImplicitNode(pair.first)) {
        claimed = true;
        pair.second = true;
      }
    }
    return claimed;
  }

  /// \brief Checks whether this `GraphObserver` should emit data for
  /// nodes at some `SourceLocation`.
  ///
  /// \param Loc The location to claim.
  ///
  /// It is always safe to return `true` here (as this will result in
  /// redundant output instead of dropped output).
  virtual bool claimLocation(clang::SourceLocation Loc) { return true; }

  /// \brief Checks whether this `GraphObserver` should emit data for
  /// nodes contained by some `Range`.
  ///
  /// \param Range The range to claim.
  ///
  /// It is always safe to return `true` here (as this will result in
  /// redundant output instead of dropped output).
  virtual bool claimRange(const Range& R) { return true; }

  /// \brief Returns a `ClaimToken` for intrinsics.
  ///
  /// By default, this will return the default claim token. This does not
  /// associate the builtin with any particular location and ensures that
  /// information for the builtin will be emitted when it is used in some
  /// relationship.
  virtual const ClaimToken* getClaimTokenForBuiltin() const {
    return getDefaultClaimToken();
  }

  /// \brief Returns a `ClaimToken` covering a given source location.
  ///
  /// NB: FileIds represent each *inclusion* of a file. This allows us to
  /// map from the FileId inside a SourceLocation to a (file, transcript)
  /// pair.
  virtual const ClaimToken* getClaimTokenForLocation(
      const clang::SourceLocation L) const {
    return getDefaultClaimToken();
  }

  /// \brief Returns a `ClaimToken` covering a given source range.
  virtual const ClaimToken* getClaimTokenForRange(
      const clang::SourceRange& SR) const {
    return getDefaultClaimToken();
  }

  /// \brief Append a string representation of `Range` to `Ostream`.
  virtual void AppendRangeToStream(llvm::raw_ostream& Ostream,
                                   const Range& Range) const {
    Range.PhysicalRange.getBegin().print(Ostream, *SourceManager);
    Ostream << "@";
    Range.PhysicalRange.getEnd().print(Ostream, *SourceManager);
    if (Range.Kind == Range::RangeKind::Wraith) {
      Ostream << "@";
      Ostream << Range.Context.ToClaimedString();
    }
  }

  /// \brief Create a NodeId that points to some VName.
  NodeId MintNodeIdForVName(const proto::VName& vname);

  /// \brief Creates a VNameRef for the VName stored in `id`.
  ///
  /// Note that the `VNameRef` should not outlive `id`.
  VNameRef DecodeMintedVName(const NodeId& id) const;

  /// \brief Return a vector of loaded metadata files.
  virtual std::vector<std::pair<clang::FileID, const MetadataFile*>>
  GetMetadataFiles() const {
    return {};
  }

  virtual ~GraphObserver() = default;

  clang::SourceManager* getSourceManager() const { return SourceManager; }

  const clang::LangOptions* getLangOptions() const { return LangOptions; }

  clang::Preprocessor* getPreprocessor() const { return Preprocessor; }

  const ProfilingCallback& getProfilingCallback() const {
    return ReportProfileEvent;
  }

  /// \brief Calls `iter` for each claimed FileID.
  ///
  /// If `iter` returns false, terminates iteration.
  virtual void iterateOverClaimedFiles(
      std::function<bool(clang::FileID, const NodeId&)> iter) const {}

  /// Name of the platform or build configuration to emit on anchors.
  virtual absl::string_view getBuildConfig() const { return ""; }

 protected:
  clang::SourceManager* SourceManager = nullptr;
  const clang::LangOptions* LangOptions = nullptr;
  clang::Preprocessor* Preprocessor = nullptr;
  ProfilingCallback ReportProfileEvent = [](const char*, ProfilingEvent) {};
  HashRecorder* hash_recorder_ = nullptr;
};

/// \brief A GraphObserver that does nothing.
class NullGraphObserver : public GraphObserver {
 public:
  /// \brief A ClaimToken that provides no information or evidence.
  class NullClaimToken : public ClaimToken {
   public:
    std::string StampIdentity(const std::string& Identity) const override {
      return Identity;
    }
    uintptr_t GetClass() const override { return kNullClaimTokenClass; }
    bool operator==(const ClaimToken& RHS) const override {
      return RHS.GetClass() == GetClass();
    }
    bool operator!=(const ClaimToken& RHS) const override {
      return RHS.GetClass() != GetClass();
    }

   private:
    static inline const uintptr_t kNullClaimTokenClass =
        reinterpret_cast<uintptr_t>(&kNullClaimTokenClass);
  };

  NodeId getNodeIdForBuiltinType(llvm::StringRef Spelling) const override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId nodeIdForTypeAliasNode(const NameId& AliasName,
                                const NodeId& AliasedType) const override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId recordTypeAliasNode(
      const NodeId& AliasId, const NodeId& AliasedType,
      const std::optional<NodeId>& RootAliasedType,
      const std::optional<MarkedSource>& MarkedSource) override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId nodeIdForNominalTypeNode(const NameId& type_name) const override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId recordNominalTypeNode(const NodeId& TypeNode,
                               const std::optional<MarkedSource>& MarkedSource,
                               const std::optional<NodeId>& Parent) override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId nodeIdForTsigmaNode(absl::Span<const NodeId> Params) const override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId recordTsigmaNode(const NodeId& TsigmaId,
                          absl::Span<const NodeId> Params) override {
    return TsigmaId;
  }

  NodeId nodeIdForTappNode(const NodeId& TyconId,
                           absl::Span<const NodeId> Params) const override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  NodeId recordTappNode(const NodeId& TappId, const NodeId& TyconId,
                        absl::Span<const NodeId> Params,
                        unsigned FirstDefaultParam) override {
    return NodeId::CreateUncompressed(getDefaultClaimToken(), "");
  }

  const ClaimToken* getDefaultClaimToken() const override {
    return &DefaultToken;
  }

  const ClaimToken* getVNameClaimToken() const override { return &VnameToken; }

  ~NullGraphObserver() {}

 private:
  NullClaimToken DefaultToken;

  NullClaimToken VnameToken;
};

/// \brief Emits a stringified representation of the given `NameId`,
/// including its `NameEqClass`, to the given stream.
inline llvm::raw_ostream& operator<<(llvm::raw_ostream& OS,
                                     const GraphObserver::NameId& N) {
  OS << N.Path << "#";
  switch (N.EqClass) {
    case GraphObserver::NameId::NameEqClass::None:
      OS << "n";
      break;
    case GraphObserver::NameId::NameEqClass::Class:
      OS << "c";
      break;
    case GraphObserver::NameId::NameEqClass::Union:
      OS << "u";
      break;
    case GraphObserver::NameId::NameEqClass::Macro:
      OS << "m";
      break;
  }
  return OS;
}

/// \brief Emits a stringified representation of the given `NodeId`.
inline llvm::raw_ostream& operator<<(llvm::raw_ostream& OS,
                                     const GraphObserver::NodeId& N) {
  return OS << N.ToClaimedString();
}

inline std::string GraphObserver::NameId::ToString() const {
  std::string Rep;
  llvm::raw_string_ostream Ostream(Rep);
  Ostream << *this;
  return Ostream.str();
}

inline bool operator==(const GraphObserver::Range& L,
                       const GraphObserver::Range& R) {
  return L.Kind == R.Kind && L.PhysicalRange == R.PhysicalRange &&
         (L.Kind == GraphObserver::Range::RangeKind::Physical ||
          L.Context == R.Context);
}

inline bool operator!=(const GraphObserver::Range& L,
                       const GraphObserver::Range& R) {
  return !(L == R);
}

/// Returns a compact string representation of the `Hash`.
inline std::string HashToString(size_t Hash) {
  // 64 characters that can appear in identifiers (plus $ from Java).
  static constexpr char kSafeEncodingCharacters[] =
      "abcdefghijklmnopqrstuvwxyz012345"
      "6789_$ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static constexpr size_t kBitsPerCharacter = 6;
  static_assert((1 << kBitsPerCharacter) == sizeof(kSafeEncodingCharacters) - 1,
                "The alphabet is big enough");

  if (!Hash) {
    return "";
  }
  int SetBit = llvm::bit_width(Hash);
  size_t Pos = (SetBit + kBitsPerCharacter - 1) / kBitsPerCharacter;
  std::string HashOut(Pos, kSafeEncodingCharacters[0]);
  while (Hash) {
    HashOut[--Pos] =
        kSafeEncodingCharacters[Hash & ((1 << kBitsPerCharacter) - 1)];
    Hash >>= kBitsPerCharacter;
  }
  return HashOut;
}

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_GRAPH_OBSERVER_H_
