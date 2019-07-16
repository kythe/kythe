# TypeScript Indexer VName Schema

This specification defines the schema for how the indexer expresses VNames for
TypeScript declarations. You may find this useful if you are developing an
application that relies on TypeScript code you don't want to re-index.

A formal listing of the specification is provided below, but the
[schema tests](./testdata/schema.ts) may be more useful for real examples.

### VName signature

The signature description language used in this document is similar to regex.

-   A substring in `\b\$.*\b` is a variable.
    -   e.g. `$DECLARATION_NAME` is a variable refering to the name of
        declaration.
-   A substring in `\[.*\]\?` is matched 0 or 1 times.
    -   e.g. `(hidden)?` is really `hidden` or ``.
-   A substring in `\[.*\]\*` is matched 0 or more times.
    -   e.g. `[a]*` is ``, or`a`, or`aa`, ...
-   All other substrings are literals.
    -   e.g. `get#$NAME` is really `get#foo` if `$NAME = foo`.

The signature of a TypeScript declaration is defined by the following schema:

```regex
$PART[.$PART]*[#type]?
```

where `$PART` is a component of the enclosing declaration scope and `#type` is
appended to the signature of types. `SyntaxKind`s that are types are:

-   `ClassDeclaration` (also a value)
-   `EnumDeclaration` (also a value)
-   `InterfaceDeclaration`
-   `TypeAliasDeclaration`
-   `TypeParameter`

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
describing the module. The module path is the file path of the module, stripped
of `.ts` and `.d.ts` extensions and relative to the project root. See __VName
Path__ for more.

**SyntaxKind**:

-   `SourceFile`

```typescript
//- FileModule=VName("module", _, _, _, _).node/kind record
//- FileModuleAnchor.node/kind anchor
//- FileModuleAnchor./kythe/loc/start 0
//- FileModuleAnchor./kythe/loc/end 1
//- FileModuleAnchor defines/binding FileModule
```

#### Named Declaration

**Form**: `$DECLARATION_NAME`

**Notes**: Every named declaration is guaranteed to have a signature of the form
`$SCOPE[.$SCOPE]*` where `$SCOPE` is the name of each encompassing named
declaration.

**SyntaxKind**:

-   `NamespaceImport`
-   `Constructor`
-   `MethodDeclaration`
-   `EnumDeclaration`
-   `EnumMember`
-   `FunctionDeclaration`
-   `Parameter`
-   `InterfaceDeclaration`
-   `PropertySignature`
-   `MethodSignature`
-   `VariableDeclaration`
-   `PropertyAssignment`
-   `TypeAliasDeclaration`
-   `TypeParameter`

```typescript
export class Klass {
  //- @property defines/binding VName("Klass#type.method", _, _, _, _)
  method() {
    //- @property defines/binding VName("Klass#type.method.val", _, _, _, _)
    let val;
  };
}
```

#### Class Declaration

**Form**:

-   `$CLASS#type`
-   `$CLASS`

**Notes**: Because a class is both a type and a value, it has to be labeled as
such. Class members belong to an instance of the class type and have a signature
with class component of `$CLASS#type`. Static members belong to the class value
and have a signature with class component of `$CLASS`.

**SyntaxKind**:

-   ClassDeclaration

```typescript
//- @Klass defines/binding VName("Klass#type", _, _, _, _)
//- @Klass defines/binding VName("Klass", _, _, _, _)
class Klass {
  //- @member defines/binding VName("Klass#type.member", _, _, _, _)
  member = 0;

  //- @staticMember defines/binding VName("Klass.staticMember", _, _, _, _)
  static staticMember = 0;

  //- @constructor defines/binding VName("Klass#type.constructor", _, _, _, _)
  constructor() {}
}
```

#### Property Declaration

**Form**: `$CLASS[#type]?.$PROPERTY`

**Notes**: Instance members on a class have a signature of form
`$CLASS#type.$PROPERTY`, as they belong to instances of the class type. Static
members, which belong to the class value, have a form of `$CLASS.$PROPERTY`.

**SyntaxKind**:

-   PropertyDeclaration
-   ParameterPropertyDeclaration
-   GetAccessor
-   SetAccessor
-   MethodDeclaration

```typescript
class Klass {
  //- @prop defines/binding VName("Klass.prop", _, _, _, _)
  prop;

  //- @prop defines/binding VName("Klass#type.prop", _, _, _, _)
  static prop;

  //- @cprop defines/binding VName("Klass#type.cprop", _, _, _, _)
  constructor(private cprop: number) {}
}
```

#### Getter/Setter

**Form**:

-   getter: `$DECLARATION_NAME:getter`
-   setter: `$DECLARATION_NAME:setter`
-   getter if present, otherwise setter: `$DECLARATION_NAME`

**Notes**: Because getters and setters refer to the same property on a class but
provide multiple declarations, several distinctions between the declarations are
made. Each declaration of a getter and setter will take `:getter` and `:setter`
suffix, respectively.

If a getter for a property is present, the getter will emit a binding the same
form as the property it defines. If only a setter is present, the setter will
emit a binding of the same form as the property it defines.

**SyntaxKind**:

-   GetAccessor
-   SetAccessor

```typescript
class Klass {
  //- @foo defines/binding VName("Klass.foo:getter", _, _, _, _)
  //- @foo defines/binding VName("Klass.foo", _, _, _, _)
  get foo() {}
  //- @foo defines/binding VName("Klass.foo:setter", _, _, _, _)
  set foo(newFoo) {}
}

class KlassNoGetter {
  //- @foo defines/binding VName("KlassNoGetter.foo:setter", _, _, _, _)
  //- @foo defines/binding VName("KlassNoGetter.foo", _, _, _, _)
  set foo(newFoo) {}
}
```

#### Export Assignment

**Form**:

- default export: `default`
- equals export: `export=`

**Notes**: `export default` is semantically equivalent to exporting a variable
named `default`. `export =` exports a variable named `export=`.

**SyntaxKind**:

-   `ExportAssignment`

```typescript
//- @default defines/binding VName("default", _, _, _, _)
export default myExport;
//- @"export =" defines/binding VName("export=", _, _, _, _)
export = { myExport };
```

#### Anonymous Block

**Form**: a unique, non-deterministic block name.

**Notes**: The block name is not guaranteed, because declarations within an
anonymous block cannot be accessed outside it.

**SyntaxKind**:

-   `ArrowFunction`
-   `Block` that does not have a `FunctionDeclaration` or `MethodDeclaration`
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

See the module name discussion in the [README](./README.md). In short,

-   For file nodes
    -   the entire file path, relative to the corpus and root. A file `a/b/c.ts`
        will have a file node with path `a/b/c.ts`.
-   For TypeScript nodes
    -   the file path stripped of `.d.ts` or `.ts` extensions, relative to the
        project root. A TypeScript anchor in a file `a/b/c.ts` with project root
        `a/` will have a node with path of `b/c`.

### VName language

Always `typescript`.
