# Kythe indexer for TypeScript

## Development

### Building code

```shell
cd kythe/typescript
bazel test :all
```


### Running tests

To run tests use:

```shell
cd kythe/typescript
bazel test :indexer_test
```

To run single test from file `testdata/foo.ts`:

```shell
bazel test --test_arg=foo :indexer_test
```

### Writing tests

By default in TypeScript, files are "scripts", where every declaration is in the
global scope. If the file has any `import` or `export` declaration, they become
a "module", where declarations are local. To make tests isolated from one
another, prefix each test with an `export {}` to make them modules. In larger
TypeScript projects this doesn't come up because all files are modules.

### Build system and dependencies

We use [aspect-build/rules_ts](https://github.com/aspect-build/rules_ts) to handle
TypeScript rules. Dependencies are specified in `package.json` file whick you can find
in the top-level directory of this repo and not in the current package.

We don't use `package-lock.json` and instead use use [pnpm](https://pnpm.io/motivation)
(pnpm-lock.yaml in the top-level directory) to pin dependencies. When updating
dependencies in `package.json` you need to update `pnpm-lock.yaml` by
running `npx pnpm update`.

## Design notes

### Plugin system

Indexer is based on plugin system. Each plugin takes an `IndexerHost` instance
and emits Kythe nodes, facts and edges. Plugin usually iterates through a set of
srcs files and processes them resulting in Kythe data.

There is one default plugin that always included - `TypescriptIndexer`. This
plugin does the main indexing work: go through all symbols in file and emit
nodes and edges for them. Other plugins live outside of this repo and are can
be passed as optional array.

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
to the *declarations* of the `Symbol`, which then point to syntactic positions
(like the `function` above), and then from there maps the declaration back to
the containing scopes to choose a unique name.

This seems to work so far but we might need to revisit it. I'm not yet clear on
whether this approach is correct for symbols with declarations in multiple
modules, for example.

### Module name

A file `foo/bar.ts` has an associated *module name* `foo/bar`. This is distinct
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
