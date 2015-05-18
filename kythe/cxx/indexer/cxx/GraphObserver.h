/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// This file uses the Clang style conventions.

#ifndef KYTHE_CXX_INDEXER_CXX_GRAPH_OBSERVER_H_
#define KYTHE_CXX_INDEXER_CXX_GRAPH_OBSERVER_H_

/// \file
/// \brief Defines the class kythe::GraphObserver

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Lex/Preprocessor.h"

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {

// TODO(zarko): Most of the documentation for this interface belongs here.

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
    virtual std::string StampIdentity(const std::string &Identity) const = 0;
    /// \brief Returns a value unique to each implementation of `ClaimToken`.
    virtual void *GetClass() const = 0;
    /// \brief Checks for equality.
    ///
    /// `ClaimTokens` are only equal if they have the same value for `GetClass`.
    virtual bool operator==(const ClaimToken &RHS) const = 0;
    virtual bool operator!=(const ClaimToken &RHS) const = 0;
  };

  /// \brief The identifier for an object in the graph being observed.
  ///
  /// A node is identified uniquely in the graph by its `Token`, which
  /// provides evidence of its provenance (and may be used to determine whether
  /// the node should be analyzed), and its `Identity`, a string of bytes
  /// determined by the `IndexerASTHooks` and `GraphObserver` override.
  class NodeId {
  public:
    explicit NodeId(const ClaimToken *Token) : Token(Token) {}
    NodeId(const NodeId &C) { *this = C; }
    NodeId &operator=(const NodeId *C) {
      Token = C->Token;
      Identity = C->Identity;
      return *this;
    }
    /// \brief Returns a string representation of this `NodeId`.
    std::string ToString() const { return Identity; }
    /// \brief Returns a string representation of this `NodeId`
    /// annotated by its claim token.
    std::string ToClaimedString() const {
      return Token->StampIdentity(Identity);
    }
    bool operator==(const NodeId &RHS) const {
      return *Token == *RHS.Token && Identity == RHS.Identity;
    }
    bool operator!=(const NodeId &RHS) const {
      return *Token != *RHS.Token || Identity != RHS.Identity;
    }
    const ClaimToken *Token;
    std::string Identity;
  };

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
      /// This range is related to some bytes, but also lives in an
      /// imaginary context; for example, a declaration of a member variable
      /// inside an implicit template instantiation has its source range
      /// set to the declaration's range in the template being instantiated
      /// and its context set to the `NodeId` of the implicit specialization.
      Wraith
    };
    /// \brief Constructs a physical `Range` for the given `clang::SourceRange`.
    Range(const clang::SourceRange &R, const ClaimToken *T)
        : Kind(RangeKind::Physical), PhysicalRange(R), Context(T) {}
    /// \brief Constructs a `Range` with some physical location, but specific to
    /// the context of some semantic node.
    Range(const clang::SourceRange &R, const NodeId &C)
        : Kind(RangeKind::Wraith), PhysicalRange(R), Context(C) {}
    /// \brief Constructs a new `Range` in the context of an existing
    /// `Range`, but with a different physical location.
    Range(const Range &R, const clang::SourceRange &NR)
        : Kind(R.Kind), PhysicalRange(NR), Context(R.Context) {}

    RangeKind Kind;
    clang::SourceRange PhysicalRange;
    NodeId Context;
  };

  struct NameId {
    /// \brief C++ distinguishes between several equivalence classes of names,
    /// a selection of which we consider important to represent.
    enum class NameEqClass {
      None,  ///< This name is not a member of a significant class.
      Union, ///< This name names a union record.
      Class, ///< This name names a non-union class or struct.
      Macro  ///< This name names a macro.
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
    /// \brief Returns a string representation of this `NameId`.
    std::string ToString() const;
  };

  /// \brief Determines whether an edge an opt out of claiming.
  ///
  /// If claiming based on the preprocessor is not enough to determine whether
  /// an edge adds unique content to the graph, that edge may be marked as
  /// Unclaimable.
  enum class Claimability {
    Claimable,  ///< This edge may be dropped by claiming.
    Unclaimable ///< This edge must always be emitted.
  };

  GraphObserver() {}

  /// \brief Sets the `SourceManager` that this `GraphObserver` should use.
  ///
  /// Since the `SourceManager` may not exist at the time the
  /// `GraphObserver` is created, it must be set after construction.
  ///
  /// \param SM the context for all `SourceLocation` instances.
  virtual void setSourceManager(clang::SourceManager *SM) {
    SourceManager = SM;
  }

  /// \param LO the language options in use.
  virtual void setLangOptions(clang::LangOptions *LO) { LangOptions = LO; }

  /// \param PP The `Preprocessor` to use.
  virtual void setPreprocessor(clang::Preprocessor *PP) { Preprocessor = PP; }

  /// \brief Returns a claim token that provides no additional information.
  virtual const ClaimToken *getDefaultClaimToken() const = 0;

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
  virtual NodeId getNodeIdForBuiltinType(const llvm::StringRef &Spelling) = 0;

  /// \brief Returns the ID for a type node aliasing another type node.
  /// \param AliasName a `NameId` for the alias name.
  /// \param AliasedType a `NodeId` corresponding to the aliased type.
  /// \return the `NodeId` for the type node corresponding to the alias.
  virtual NodeId nodeIdForTypeAliasNode(const NameId &AliasName,
                                        const NodeId &AliasedType) = 0;

  /// \brief Records a type alias node (eg, from a `typedef` or
  /// `using Alias = ty` instance).
  /// \param AliasName a `NameId` for the alias name.
  /// \param AliasedType a `NodeId` corresponding to the aliased type.
  /// \return the `NodeId` for the type alias node this definition defines.
  virtual NodeId recordTypeAliasNode(const NameId &AliasName,
                                     const NodeId &AliasedType) = 0;

  /// \brief Returns the ID for a nominal type node (such as a struct,
  /// typedef or enum).
  /// \param TypeName a `NameId` corresponding to a nominal type.
  /// \return the `NodeId` for the type node corresponding to `TypeName`.
  virtual NodeId nodeIdForNominalTypeNode(const NameId &TypeName) = 0;

  /// \brief Records a type node for some nominal type (such as a struct,
  /// typedef or enum), returning its ID.
  /// \param TypeName a `NameId` corresponding to a nominal type.
  /// \return the `NodeId` for the type node corresponding to `TypeName`.
  virtual NodeId recordNominalTypeNode(const NameId &TypeName) = 0;

  /// \brief Records a type application node, returning its ID.
  /// \note This is the elimination form for the `abs` node.
  /// \param TyconId The `NodeId` of the appropriate type constructor or
  /// abstraction.
  /// \param Params The `NodeId`s of the types to apply to the constructor or
  /// abstraction.
  /// \return The application's result's ID.
  virtual NodeId recordTappNode(const NodeId &TyconId,
                                const std::vector<const NodeId *> &Params) = 0;

  enum class RecordKind { Struct, Class, Union };

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

  /// \brief Records a node representing a record type (such as a class or
  /// struct).
  /// \param Node The NodeId of the record.
  /// \param Kind Whether this record is a struct, class, or union.
  /// \param RecordCompleteness Whether the record is complete.
  virtual void recordRecordNode(const NodeId &Node, RecordKind Kind,
                                Completeness RecordCompleteness) {}

  /// \brief Records a node representing a function.
  /// \param Node The NodeId of the function.
  /// \param FunctionCompleteness Whether the function is complete.
  virtual void recordFunctionNode(const NodeId &Node,
                                  Completeness FunctionCompleteness) {}

  /// \brief Records a node representing a callable, an object that can
  /// appear as the target of a call expression.
  /// \param Node The NodeId of the callable.
  ///
  /// Various language-level objects may be deemed 'callable' (like functions
  /// or classes with operator()). This abstraction allows call relationships
  /// to be recorded in the graph in a consistent way regardless of the
  /// particular kind of object being called.
  ///
  /// \sa recordCallableAsEdge
  virtual void recordCallableNode(const NodeId &Node) {}

  /// \brief Describes whether an enum is scoped (`enum class`).
  enum class EnumKind {
    Scoped,  ///< This enum is scoped (an `enum class`).
    Unscoped ///< This enum is unscoped (a plain `enum`).
  };

  /// \brief Records a node representing a dependent type abstraction, like
  /// a template.
  ///
  /// Abstraction nodes are used to represent the binding sites of compile-time
  /// variables. Consider the following class template definition:
  ///
  /// ~~~
  /// template <typename T> class C { T m; };
  /// ~~~
  ///
  /// Here, the `GraphObserver` will be notified of a single abstraction
  /// node. This node will have a single parameter, recorded with
  /// `recordAbsVarNode`. The abstraction node will have a child record
  /// node, which in turn will have a field `m` with a type that depends on
  /// the abstraction variable parameter.
  ///
  /// \param Node The NodeId of the abstraction.
  /// \sa recordAbsVarNode
  virtual void recordAbsNode(const NodeId &Node) {}

  /// \brief Records a node representing a variable in a dependent type
  /// abstraction.
  /// \param Node The `NodeId` of the variable.
  /// \sa recordAbsNode
  virtual void recordAbsVarNode(const NodeId &Node) {}

  /// \brief Records a node representing a deferred lookup.
  /// \param Node The `NodeId` of the lookup.
  /// \param Name The `Name` for which resolution has been deferred.
  virtual void recordLookupNode(const NodeId &Node,
                                const llvm::StringRef &Name) {}

  /// \brief Records a parameter relationship.
  /// \param `ParamOfNode` The node this `ParamNode` is the parameter of.
  /// \param `Ordinal` The ordinal for the parameter (0 is the first).
  /// \param `ParamNode` The `NodeId` for the parameter.
  virtual void recordParamEdge(const NodeId &ParamOfNode, uint32_t Ordinal,
                               const NodeId &ParamNode) {}

  /// \brief Records a node representing an enumerated type.
  /// \param Compl Whether the enum is complete.
  /// \param EnumKind Whether the enum is scoped.
  virtual void recordEnumNode(const NodeId &Node, Completeness Compl,
                              EnumKind Kind) {}

  /// \brief Records a node representing a constant with a value representable
  /// with an integer.
  ///
  /// Note that the type of the language-level object this node represents may
  /// not be integral. For example, `recordIntegerConstantNode` is used to
  /// record information about the implicitly or explicitly assigned values
  /// for enumerators.
  virtual void recordIntegerConstantNode(const NodeId &Node,
                                         const llvm::APSInt &Value) {}
  // TODO(zarko): Handle other values. How should we represent class
  // constants?

  /// \brief Records that a variable (either local or global) has been
  /// declared.
  /// \param DeclName The name to which this element is being bound.
  /// \param DeclNode The identifier for this particular element.
  /// \param Compl The completeness of this variable declaration.
  // TODO(zarko): We should make note of the storage-class-specifier (dcl.stc)
  // of the variable, which is a property the variable itself and not of its
  // type.
  virtual void recordVariableNode(const NameId &DeclName,
                                  const NodeId &DeclNode, Completeness Compl) {}

  // TODO(zarko): recordExpandedTypeEdge -- records that a type was seen
  // to have some canonical type during a compilation. (This is a 'canonical'
  // type for a given compilation, but may differ between compilations.)

  /// \brief Records that a particular `Range` contains the definition
  /// of the node called `DefnId`.
  ///
  /// Generally the `Range` provided will be small and limited only to
  /// the part of the declaration that binds a name. For example, in `class C`,
  /// we would `recordDefinitionRange` on the range for `C`.
  virtual void recordDefinitionRange(const Range &SourceRange,
                                     const NodeId &DefnId) {}

  /// \brief Describes how specific a completion relationship is.
  enum class Specificity {
    /// This relationship is the only possible relationship given its context.
    /// For example, a class definition uniquely completes a forward declaration
    /// in the same source file.
    UniquelyCompletes,
    /// This relationship is one of many possible relationships. For example, a
    /// forward declaration in a header file may be completed by definitions in
    /// many different source files.
    Completes
  };

  /// \brief Records that a particular `Range` contains a completion
  /// for the node named `DefnId`.
  /// \param SourceRange The source range containing the completion.
  /// \param DefnId The `NodeId` for the node being completed.
  /// \param Spec the specificity of the relationship beween the `Range`
  /// and the `DefnId`.
  virtual void recordCompletionRange(const Range &SourceRange,
                                     const NodeId &DefnId, Specificity Spec) {}

  /// \brief Records that a particular `Node` has been given some `Name`.
  ///
  /// A given node may have zero or more names distinct from its `NodeId`.
  /// These may be used as entry points into the graph. For example,
  /// `recordNamedEdge` may be called with the unique `NodeId` for a function
  /// definition and the `NameId` corresponding to the fully-qualified name
  /// of that function; it may subsequently be called with that same `NodeId`
  /// and a `NameId` corresponding to that function's mangled name.
  ///
  /// A call to `recordNamedEdge` may have the following form:
  /// ~~~
  /// // DeclName is the lookup name for some record type (roughly, the "C" in
  /// // "class C").
  /// GraphObserver::NameId DeclName = BuildNameIdForDecl(RecordDecl);
  /// // DeclNode refers to a particular decl of some record type.
  /// GraphObserver::NodeId DeclNode = BuildNodeIdForDecl(RecordDecl);
  /// Observer->recordNamedEdge(DeclNode, DeclName);
  /// ~~~
  virtual void recordNamedEdge(const NodeId &Node, const NameId &Name) {}

  /// \brief Records the type of a node as an edge in the graph.
  /// \param TermNodeId The identifier for the node to be given a type.
  /// \param TypeNodeId The identifier for the node representing the type.
  virtual void recordTypeEdge(const NodeId &TermNodeId,
                              const NodeId &TypeNodeId) {}

  /// \brief Records that some term specializes some abstraction.
  /// \param TermNodeId The identifier for the node specializing the
  /// abstraction.
  /// \param TypeNodeId The identifier for the node representing the specialized
  /// abstraction.
  virtual void recordSpecEdge(const NodeId &TermNodeId,
                              const NodeId &AbsNodeId) {}

  /// \brief Records that some term instantiates some abstraction.
  /// \param TermNodeId The identifier for the node instantiating the type.
  /// \param TypeNodeId The identifier for the node representing the
  /// instantiated abstraction.
  virtual void recordInstEdge(const NodeId &TermNodeId,
                              const NodeId &AbsNodeId) {}

  /// \brief Records that one node participates in the call graph as a
  /// particular `Callable`.
  ///
  /// This relationship allows the indexer to abstract the notion of
  /// callability from its embodiment in particular callable objects
  /// (such as functions or records that define operator()).
  ///
  /// \param ToCallId The specific node that may be called.
  /// \param CallableAsId The node representing `ToCallId` in the call graph.
  virtual void recordCallableAsEdge(const NodeId &ToCallId,
                                    const NodeId &CallableAsId) {}

  /// \brief Records that a callable is called at a particular location.
  /// \param CallLoc The `Range` responsible for making the call.
  /// \param CallerId The scope to be held responsible for making the call;
  /// for example, a function.
  /// \param CalleeId The callable being called.
  virtual void recordCallEdge(const Range &SourceRange, const NodeId &CallerId,
                              const NodeId &CalleeId) {}

  /// \brief Records a child-to-parent relationship as an edge in the graph.
  /// \param ChildNodeId The identifier for the child node.
  /// \param ParentNodeId The identifier for the parent node.
  virtual void recordChildOfEdge(const NodeId &ChildNodeId,
                                 const NodeId &ParentNodeId) {}

  /// \brief Records that a record directly inherits from another record.
  /// \param InheritingNodeId The inheriting record type (the derived class).
  /// \param InheritedTypeId The record type being inherited (the base class).
  /// \param IsVirtual True if the inheritance is virtual.
  /// \param AS The access specifier for this inheritance.
  virtual void recordExtendsEdge(const NodeId &InheritingNodeId,
                                 const NodeId &InheritedTypeId, bool IsVirtual,
                                 clang::AccessSpecifier AS) {}

  /// \brief Records a node with a provided kind.
  /// \param Name The name of the node to record (including a `named` edge from
  /// `Id`).
  /// \param Id The ID of the node to record.
  /// \param NodeKind The kind of the node ("google/protobuf")
  /// \param Compl Whether this node is complete.
  virtual void recordUserDefinedNode(const NameId &Name, const NodeId &Id,
                                     const llvm::StringRef &NodeKind,
                                     Completeness Compl) {}

  /// \brief Records a use site for some decl.
  virtual void
  recordDeclUseLocation(const Range &SourceRange, const NodeId &DeclId,
                        Claimability Cl = Claimability::Claimable) {}

  /// \brief Records that a type was spelled out at a particular location.
  /// \param SourceRange The source range covering the type spelling.
  /// \param TypeNode The identifier for the type being spelled out.
  /// \param Cr Whether this information can be dropped by claiming.
  virtual void
  recordTypeSpellingLocation(const Range &SourceRange, const NodeId &TypeId,
                             Claimability Cl = Claimability::Claimable) {}

  /// \brief Records that a macro was defined.
  virtual void recordMacroNode(const NodeId &MacroNode) {}

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
  virtual void recordExpandsRange(const Range &SourceRange,
                                  const NodeId &MacroId) {}

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
  virtual void recordIndirectlyExpandsRange(const Range &SourceRange,
                                            const NodeId &MacroId) {}

  /// \brief Records that a macro was undefined at some location.
  /// \param SourceRange The `Range` covering the text causing the undefinition.
  /// \param MacroId The `NodeId` of the macro being undefined.
  virtual void recordUndefinesRange(const Range &SourceRange,
                                    const NodeId &MacroId) {}

  /// \brief Records that a defined macro was queried at some location.
  ///
  /// Testing a macro for definedness does not expand it.
  ///
  /// \param SourceRange The `Range` covering the text causing the query.
  /// \param MacroId The `NodeId` of the macro being queried.
  virtual void recordBoundQueryRange(const Range &SourceRange,
                                     const NodeId &MacroId) {}

  /// \brief Records that an undefined macro was queried at some location.
  ///
  /// Testing a macro for definedness does not expand it.
  ///
  /// \param SourceRange The `Range` covering the text causing the query.
  /// \param MacroName The `NameId` of the macro being queried.
  virtual void recordUnboundQueryRange(const Range &SourceRange,
                                       const NameId &MacroName) {}

  /// \brief Records that another resource was included at some location.
  /// \param SourceRange The `Range` covering the text causing the inclusion.
  /// \param File The resource being included.
  virtual void recordIncludesRange(const Range &SourceRange,
                                   const clang::FileEntry *File) {}

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

  /// \brief Checks whether this `GraphObserver` should emit data for some
  /// `NodeId` and its descendants.
  ///
  /// \param NodeId The node to claim.
  ///
  /// It is always safe to return `true` here (as this will result in
  /// redundant output instead of dropped output).
  virtual bool claimNode(const NodeId &NodeId) { return true; }

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
  virtual bool claimRange(const Range &R) { return true; }

  /// \brief Returns a `ClaimToken` for intrinsics.
  ///
  /// By default, this will return the default claim token. This does not
  /// associate the builtin with any particular location and ensures that
  /// information for the builtin will be emitted when it is used in some
  /// relationship.
  virtual const ClaimToken *getClaimTokenForBuiltin() {
    return getDefaultClaimToken();
  }

  /// \brief Returns a `ClaimToken` covering a given source location.
  ///
  /// NB: FileIds represent each *inclusion* of a file. This allows us to
  /// map from the FileId inside a SourceLocation to a (file, transcript)
  /// pair.
  virtual const ClaimToken *
  getClaimTokenForLocation(const clang::SourceLocation L) {
    return getDefaultClaimToken();
  }

  /// \brief Returns a `ClaimToken` covering a given source range.
  virtual const ClaimToken *
  getClaimTokenForRange(const clang::SourceRange &SR) {
    return getDefaultClaimToken();
  }

  /// \brief Append a string representation of `Range` to `Ostream`.
  /// \return true on success, false if the range was invalid.
  virtual bool AppendRangeToStream(llvm::raw_ostream &Ostream,
                                   const Range &Range) {
    if (Range.PhysicalRange.isInvalid()) {
      return false;
    }
    Range.PhysicalRange.getBegin().print(Ostream, *SourceManager);
    Ostream << "@";
    Range.PhysicalRange.getEnd().print(Ostream, *SourceManager);
    if (Range.Kind == Range::RangeKind::Wraith) {
      Ostream << "@";
      Ostream << Range.Context.ToClaimedString();
    }
    return true;
  }

  virtual ~GraphObserver() = 0;

  clang::SourceManager *getSourceManager() { return SourceManager; }

  clang::LangOptions *getLangOptions() { return LangOptions; }

  clang::Preprocessor *getPreprocessor() { return Preprocessor; }

protected:
  clang::SourceManager *SourceManager = nullptr;
  clang::LangOptions *LangOptions = nullptr;
  clang::Preprocessor *Preprocessor = nullptr;
};

inline GraphObserver::~GraphObserver() {}

/// \brief A GraphObserver that does nothing.
class NullGraphObserver : public GraphObserver {
public:
  /// \brief A ClaimToken that provides no information or evidence.
  class NullClaimToken : public ClaimToken {
  public:
    std::string StampIdentity(const std::string &Identity) const override {
      return Identity;
    }
    void *GetClass() const override { return &NullClaimTokenClass; }
    bool operator==(const ClaimToken &RHS) const override {
      return RHS.GetClass() == GetClass();
    }
    bool operator!=(const ClaimToken &RHS) const override {
      return RHS.GetClass() != GetClass();
    }

  private:
    static void *NullClaimTokenClass;
  };

  NodeId getNodeIdForBuiltinType(const llvm::StringRef &Spelling) override {
    return NodeId(getDefaultClaimToken());
  }

  NodeId nodeIdForTypeAliasNode(const NameId &AliasName,
                                const NodeId &AliasedType) override {
    return NodeId(getDefaultClaimToken());
  }

  NodeId recordTypeAliasNode(const NameId &AliasName,
                             const NodeId &AliasedType) override {
    return NodeId(getDefaultClaimToken());
  }

  NodeId nodeIdForNominalTypeNode(const NameId &type_name) override {
    return NodeId(getDefaultClaimToken());
  }

  NodeId recordNominalTypeNode(const NameId &TypeName) override {
    return NodeId(getDefaultClaimToken());
  }

  NodeId recordTappNode(const NodeId &TyconId,
                        const std::vector<const NodeId *> &Params) override {
    return NodeId(getDefaultClaimToken());
  }

  const ClaimToken *getDefaultClaimToken() const override {
    return &DefaultToken;
  }

  ~NullGraphObserver() {}

private:
  NullClaimToken DefaultToken;
};

/// \brief Emits a stringified representation of the given `NameId`,
/// including its `NameEqClass`, to the given stream.
inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                     const GraphObserver::NameId &N) {
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
inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                     const GraphObserver::NodeId &N) {
  return OS << N.ToClaimedString();
}

inline std::string GraphObserver::NameId::ToString() const {
  std::string Rep;
  llvm::raw_string_ostream Ostream(Rep);
  Ostream << *this;
  return Ostream.str();
}

inline bool operator==(const GraphObserver::Range &L,
                       const GraphObserver::Range &R) {
  return L.Kind == R.Kind && L.PhysicalRange == R.PhysicalRange &&
         (L.Kind == GraphObserver::Range::RangeKind::Physical ||
          L.Context == R.Context);
}

inline bool operator!=(const GraphObserver::Range &L,
                       const GraphObserver::Range &R) {
  return !(L == R);
}

// 64 characters that can appear in identifiers (plus $ from Java).
static constexpr char kSafeEncodingCharacters[] =
    "abcdefghijklmnopqrstuvwxyz012345"
    "6789_$ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static constexpr size_t kBitsPerCharacter = 6;
static_assert((1 << kBitsPerCharacter) == sizeof(kSafeEncodingCharacters) - 1,
              "The alphabet is big enough");

/// Returns a compact string representation of the `Hash`.
static inline std::string HashToString(size_t Hash) {
  if (!Hash) {
    return "";
  }
  int SetBit = sizeof(Hash) * CHAR_BIT - llvm::countLeadingZeros(Hash);
  size_t Pos = (SetBit + kBitsPerCharacter - 1) / kBitsPerCharacter;
  std::string HashOut(Pos, kSafeEncodingCharacters[0]);
  while (Hash) {
    HashOut[--Pos] =
        kSafeEncodingCharacters[Hash & ((1 << kBitsPerCharacter) - 1)];
    Hash >>= kBitsPerCharacter;
  }
  return HashOut;
}

} // namespace kythe

#endif // KYTHE_CXX_INDEXER_CXX_GRAPH_OBSERVER_H_
