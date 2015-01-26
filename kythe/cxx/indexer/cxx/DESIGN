Indexing C++ code
=================

We describe an interface to support writing tools that extract semantic
information from source code. These tools, called 'indexers', are generally
used as the first phase in a toolchain for generating documentation or for
providing jump-to-definition or cross-reference support in editors. An indexer
does not need access to the full richness of Clang's AST and the concomitant
complexity and instability of supplying a new `RecursiveASTVisitor`. However,
the amount of information exposed by the C API is not sufficient. To this end,
we supply an intermediate-level 'observer' that insulates an indexer from the
underlying complexities of Clang while still offering the necessary richness of
information.

Basic data types and interfaces
-------------------------------

`libclang` wraps Clang (and LLVM)'s types out of necessity. Since we provide
a C++ interface, we instead choose to make use of existing base types when
possible. We represent strings as `llvm::StringRef` and locations in source
files as `clang::SourceLocation`.

In order to receive information about the syntactic and semantic graph
of a compilation unit, an indexer must provide an instance of the
`kythe::GraphObserver` interface. Member functions on this interface are called
as the system identifies various entities of interest. The indexer should use
these events to construct its own object model of the compilation unit under
investigation. There is no need for this object model to follow Clang's. For
example, after receiving notifications that a particular class is defined, an
indexer may create an object representing just that definition and another
object representing the class itself.

The code between Clang proper and a `kythe::GraphObserver` will generate IDs
it uses to refer to language-level objects. A distinction is drawn between the
name to which an object is bound for lookup in the language (its `NameId`) and
the name that represents the object itself (its `NodeId`). This distinction
allows the indexer to handle cases like the `main` function, which may
reasonably be defined in multiple translation units within the scope of a single
project. Here, `main` has a single `NameId` (which is, appropriately, `main`,
along with some metadata describing the kind of name it is). Each definition
of `main` gets its own `NodeId`. Each declaration of `main` also gets its own
`NodeId`. The `GraphObserver` is notified of each `NodeId` once along with its
associated `NameId` and properties of each element (such as where the
definitions and declarations are spelled out in source files).

NodeIds and the preprocessor
----------------------------

We endeavor to assign the same `NodeId` to entities declared in header files
regardless of the translation unit that includes those files. Consider the
following header:

  #ifndef SOME_HEADER_H
  #define SOME_HEADER_H
  class C;
  #endif

Any indexer run over a translation unit that touches this header file should
observe the same `NodeId` for the declaration of `class C`. (They should all
observe the same `NameId` as well.) If a different header declares a `class C`,
its declaration will be assigned a different `NodeId` but the same `NameId`.

The preprocessor might make things difficult. Consider this header:

  #ifndef ANOTHER_HEADER_H
  #define ANOTHER_HEADER_H
  class C {
    ...
  #ifdef DEBUG_C
    int debug_;
  #endif
  };
  #endif

Depending on whether the external `DEBUG_C` symbol is defined, `class C` may or
may not have the `debug_` member. Without additional direction, the framework
will still assign only one `NodeId` to the definition of `class C`. This may
lead to undesirable results. Consider:

  #ifdef DEBUG_C
  #undef DEBUG_C
  #endif
  #include "another_header.h"
  C c;

Although it is not possible for the `C` in this snippet to refer to the
version of `class C` with a `debug_` member, the `NodeId` provided will be
indistinguishable from the `NodeId` for a `C` with the member present.

To check your code for troublesome uses of the preprocessor such as these, it
may be useful to look at the `modularize` tool in `clang/tools/extra`. We may
in the future expose functionality to declare a set S of preprocessor symbols
to consider as important, such that every `NodeId` from a header file that
depends on some subset of S contains a representation of the values of that
subset. We may also include structural data in the `NodeId` generated for a
class such as a digest of the class's members. The safest bet is to ensure that
code to be indexed subscribes to the requirements of `modularize`.

Representing types
------------------

Types, just like terms, are referenced by `NodeId`. The `GraphObserver`
interface permits implementations to define how a `NodeId` is derived for
a particular type. For example, in the Kythe implementation, types with
equivalent structures are given the same `NodeId`. These identifiers are
reasonable to construct external to the database so that they can be used in
search queries.

Some C++ types are built in to the language. Each `GraphObserver` implementation
must define its own mechanism for deriving `NodeId` representations of builtin
types. For example, the Kythe implementation will identify `int`'s node as
`int#builtin`'.

C++ also provides certain builtin type constructors--like functions that, given
types as arguments, produce new types. The `GraphObserver` interface handles
applications to builtin type constructors (like `*`, which is spelled `ptr`)
and applications to user-defined constructors (like templates) using the same
abstraction. A type application node combines a type constructor with its
arguments. This type application node then stands in for the result of
applying the type constructor.

Many C++ types are identified by their names:

  class C;
  C* c;

The type `C*` is a pointer to any class whose definition is bound to the name
`C`. Many times it is easy to figure out which definition to use--it's the one
included in a header file or written down elsewhere in the text of the source
file. Other times the decision may depend how the program is linked together.
We call these sorts of types--those that may exist in a translation unit as
only a name--`nominal` types.

Some C++ declarations create names that alias other names. For example,
`typedef int T` introduces the name `T` as an alias for `int`. The indexer
does not canonicalize all aliases, as this would lose important information
that the programmer had written down. Instead, aliases are recorded as
alias nodes. As they are type nodes, a `GraphObserver` must preserve the
property that the ID of an alias node should be externally constructible (given
only the name of the alias and the ID of the aliased type).

Note that this implies that the underlying graph representation should be
able to walk through alias nodes. It may also mean that this mechanism will
need to understand the properties of some C++ qualifiers (const, volatile,
and others).
