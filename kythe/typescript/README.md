# Kythe indexer for TypeScript

## VName Specification

This specification defines how indexer expresses TypeScript declarations as
VNames. You may find this useful if you are developing an application that
relies on TypeScript code you don't want to re-index.

[spec tests](./testdata/declaration_spec.ts)

### VName signature

The signature description language used in this document similar to regex.

- A substring matching `\b\$.*\b` is a variable.
  - e.g. `$DECLARATION_NAME` is a variable refering to the name of declaration.
- A substring matching `\[.*\]\?` is matched 0 or 1 times.
  - e.g. `(hidden)?` is really `hidden` or ``.
- A substring matching `\[.*\]\*` is matched 0 or more times.
  - e.g. `[a]*` is ``, or `a`, or `aa`, ...
- All other substrings are literals.
  - e.g. `get#$NAME` is really `get#foo` if `$NAME = foo`.

The signature of a TypeScript declaration is defined by the following schema:

```regex
$PART[.$PART]*[#type]?
```

where `$PART` is a component of the enclosing declaration scope and `#type` is
appended to the signature of type declarations. `SyntaxKind`s that are type
declarations are:

- `ClassDeclaration` (also a value)
- `EnumDeclaration` (also a value)
- `InterfaceDeclaration`
- `TypeAliasDeclaration`
- `TypeParameter`

As an example of this schema, in

```typescript
class A {
  public foo: string;
}

type B = A;
```

`foo` has the signature `A.foo` and `B` has the signature `B#type`.

Signature components (`$PART`s in the schema) are defined below.

#### File Module

**Form**: `module`

**Notes**: The first character of TypeScript source file binds to a VName
describing the module.

**SyntaxKind**:

- `SourceFile`

```typescript
//- FileModule=VName("module", _, _, _, _).node/kind record
//- FileModuleAnchor.node/kind anchor
//- FileModuleAnchor./kythe/loc/start 0
//- FileModuleAnchor./kythe/loc/end 1
//- FileModuleAnchor defines/binding FileModule
```

#### Named Declaration

**Form**: `$DECLARATION_NAME(#type)?`

**Notes**: Signatures for type declarations are appended with a `#type literal`.
`SyntaxKind`s that are affected by this are annotated below.

**SyntaxKind**:

- `NamespaceImport`
- `ClassDeclaration`
- `PropertyDeclaration`
- `MethodDeclaration`
- `EnumDeclaration`
- `EnumMember`
- `FunctionDeclaration`
- `Parameter`
- `InterfaceDeclaration`
- `PropertySignature`
- `MethodSignature`
- `VariableDeclaration`
- `PropertyAssignment`
- `TypeAliasDeclaration`
- `TypeParameter`

```typescript
//- @Klass defines/binding VName("Klass", _, _, _, _)
//- @Klass defines/binding VName("Klass#type", _, _, _, _)
export class Klass {
  //- @property defines/binding VName("Klass.property", _, _, _, _)
  property = {
    //- @property defines/binding VName("Klass.property.key", _, _, _, _)
    key: 0
  };
}
```

#### Constructor

**Form**: `constructor`

**SyntaxKind**:

- `Constructor`

```typescript
class Klass {
  //- @constructor defines/binding VName("Klass.constructor", _, _, _, _)
  constructor() {}
}
```

#### Getter

**Form**: `$DECLARATION_NAME#getter`

**SyntaxKind**:

- GetAccessor

```typescript
class Klass {
  //- @foo defines/binding VName("Klass.foo#getter", _, _, _, _)
  get foo() {}
}
```

#### Setter

**Form**: `$DECLARATION_NAME#setter`

**SyntaxKind**:

- SetAccessor

```typescript
class Klass {
  //- @foo defines/binding VName("Klass.foo#setter", _, _, _, _)
  set foo(newFoo) {}
}
```

#### Export Assignment

**Form**: `default`

**Notes**: Assignment to `export` is semantically equivalent to exporting a
variable named `default`.

**SyntaxKind**:

- `ExportAssignment`

```typescript
//- @myExport defines/binding VName("default", _, _, _, _)
export = myExport;
```

#### Anonymous Block

**Form**: a unique, non-deterministic block name.

**Notes**: The block name is not guaranteed, because declarations within an
anonymous block cannot be accessed outside it.

**SyntaxKind**:

- `ArrowFunction`
- `Block` that does not have a `FunctionDeclaration` or `MethodDeclaration`
  parent

```typescript
let af = () => {
  //- @decl defines/binding VName(_, _, _, _, _)
  let decl;
};
```

### VName corpus

Project-specific, defined by the compilation unit you pass to the indexer.

### VName root

Project-specific, defined by the compilation unit you pass to the indexer.

### VName path

- For entire source code files
  - the entire file path, relative to the corpus and root.
- For declarations with a file
  - the file path stripped of `.d.ts` or `.ts` extensions, relative to the
    corpus and root.

### VName language

Always `'typescript'`.

## Development

### Dependencies

Install [yarn](https://yarnpkg.com/), then run it with no arguments to download
dependencies.

You also need an install of the kythe tools like `entrystream` and `verifier`,
and point the `KYTHE` environment variable at the path to it. You can either get
these by [building Kythe](http://kythe.io/getting-started) or by downloading the
Kythe binaries from the
[Kythe releases](https://github.com/kythe/kythe/releases) page.

### Commands

Run `yarn run build` to compile the TypeScript once.

Run `yarn run watch` to start the TypeScript compiler in watch mode, which keeps
the built program up to date. Use this while developing.

Run `yarn run browse` to run the main binary, which opens the Kythe browser
against a sample file. (You might need to set `$KYTHE` to your Kythe path
first.)

Run `yarn test` to run the test suite. (You'll need to have built first.)

Run `yarn run fmt` to autoformat the source code. (Better, configure your editor
to run clang-format on save.)

### Writing tests

By default in TypeScript, files are "scripts", where every declaration is in the
global scope. If the file has any `import` or `export` declaration, they become
a "module", where declarations are local. To make tests isolated from one
another, prefix each test with an `export {}` to make them modules. In larger
TypeScript projects this doesn't come up because all files are modules.

## Design notes

### Separate compilation

The Google TypeScript build relies heavily on TypeScript's `--declaration` flag
to enable separate compilation. The way this works is that after compiling
library A, we generate -- using that flag -- the "API shape" of A into `a.d.ts`.
Then when compiling a library B that uses A, we compile `b.ts` and `a.d.ts`
together.  The Kythe process sees the same files as well.

What this means for indexing design is that a TypeScript compilation may see
only the generated shape of a module, and not its internals.  For example,
given a file like

```
class C {
  get x(): string { return 'x'; }
}
```

The generated `.d.ts` file for it describes this getter as if it was a readonly
property:

```
class C {
  readonly x: string;
}
```

In practice, what this means is that code should not assume it can to "peek into"
another module to determine the VNames of entities. Instead, when looking at
some hypothetical code that accesses the `x` member of an instance of `C`, we
should use a consistent naming scheme to refer to `x`.

### Choosing VNames

In code like:

```
let x = 3;
x;
```

the TypeScript compiler resolves the `x`s together into a single `Symbol`
object. This concept maps nicely to Kythe's `VName` concept except that
`Symbol`s do not themselves have unique names.

You might at first think that you could just, at the `let x` line, choose a name
for the `Symbol` there and then reuse it for subsequent references. But it's not
guaranteed that you syntactically encounter a definition before its use, because
code like this is legal:

```
x();
function x() {}
```

So the current approach instead starts from the `Symbol`, then from there jumps
to the _declarations_ of the `Symbol`, which then point to syntactic positions
(like the `function` above), and then from there maps the declaration back to
the containing scopes to choose a unique name.

This seems to work so far but we might need to revisit it. I'm not yet clear on
whether this approach is correct for symbols with declarations in multiple
modules, for example.

### Module name

A file `foo/bar.ts` has an associated _module name_ `foo/bar`. This is distinct
(without the extension) because it's also possible to define that module via
other file names, such as `foo/bar.d.ts`, and all such files all define into the
single extension-less namespace.

TypeScript's `rootDirs`, which merge directories into a single shared namespace
(e.g. like the way `.` and `bazel-bin` are merged in bazel), also are collapsed
when computing a module name. In the test suite we use a `fake-genfiles`
directory to recreate a `rootDirs` environment.

Semantic VNames (like an exported value) use the module name as the 'path' field
of the VName. VNames that refer specifically to the file, such as the file text,
use the real path of the file (including the extension).
