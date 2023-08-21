/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

import 'source-map-support/register';

import * as fs from 'fs';
import * as path from 'path';
import * as ts from 'typescript';

import {EdgeKind, FactName, JSONEdge, JSONFact, JSONMarkedSource, makeOrdinalEdge, MarkedSourceKind, NodeKind, OrdinalEdge, Subkind, VName} from './kythe';
import * as utf8 from './utf8';
import {CompilationUnit, Context, IndexerHost, IndexingOptions, Plugin, TSNamespace} from './plugin_api';

const LANGUAGE = 'typescript';

enum RefType {
  READ,
  WRITE,
  READ_WRITE
}

/**
 * toArray converts an Iterator to an array of its values.
 * It's necessary when running in ES5 environments where for-of loops
 * don't iterate through Iterators.
 */
function toArray<T>(it: Iterator<T>): T[] {
  const array: T[] = [];
  for (let next = it.next(); !next.done; next = it.next()) {
    array.push(next.value);
  }
  return array;
}

/**
 * stripExtension strips the .d.ts, .ts or .tsx extension from a path.
 * It's used to map a file path to the module name.
 */
function stripExtension(path: string): string {
  return path.replace(/\.(d\.)?tsx?$/, '');
}

/**
 * Determines if a node is a variable-like declaration.
 *
 * TODO(https://github.com/microsoft/TypeScript/issues/33115): Replace this with
 * a native `ts.isHasExpressionInitializer` if TypeScript ever adds it.
 */
function hasExpressionInitializer(node: ts.Node):
    node is ts.HasExpressionInitializer {
  return ts.isVariableDeclaration(node) || ts.isParameter(node) ||
      ts.isBindingElement(node) || ts.isPropertySignature(node) ||
      ts.isPropertyDeclaration(node) || ts.isPropertyAssignment(node) ||
      ts.isEnumMember(node);
}

/**
 * Determines if a node is a static member of a class.
 */
function isStaticMember(node: ts.Node, klass: ts.Declaration): boolean {
  return ts.isPropertyDeclaration(node) && node.parent === klass &&
      ((ts.getCombinedModifierFlags(node) & ts.ModifierFlags.Static) > 0);
}

function todo(sourceRoot: string, node: ts.Node, message: string) {
  const sourceFile = node.getSourceFile();
  const file = path.relative(sourceRoot, sourceFile.fileName);
  const {line, character} =
      ts.getLineAndCharacterOfPosition(sourceFile, node.getStart());
  console.warn(`TODO: ${file}:${line}:${character}: ${message}`);
}

/**
 * Convert VName to a string that can be used as key in Maps.
 */
function vnameToString(vname: VName): string {
  return `(${vname.corpus},${vname.language},${vname.path},${vname.root},${
      vname.signature})`;
}

type NamespaceAndContext = string&{__brand: 'nsctx'};
/**
 * A SymbolVNameStore stores a mapping of symbols to the (many) VNames it may
 * have. Each TypeScript symbol can be be of a different TypeScript namespace
 * and be declared in a unique context, leading to a total (`TSNamespace` *
 * `Context`) number of possible VNames for the symbol.
 *
 *              TSNamespace + Context
 *              -----------   -------
 *              TYPE          Any
 * ts.Symbol -> VALUE         Getter  -> VName
 *              NAMESPACE     Setter
 *                            ...
 *
 * The `Any` context makes no guarantee of symbol declaration disambiguation.
 * As a result, unless explicitly set for a given symbol and namespace, the
 * VName of an `Any` context is lazily set to the VName of an arbitrary context.
 */
class SymbolVNameStore {
  private readonly store =
      new Map<ts.Symbol, Map<NamespaceAndContext, Readonly<VName>>>();

  /**
   * Serializes a namespace and context as a string to lookup in the store.
   *
   * Each instance of a JavaScript object is unique, so using one as a key fails
   * because a new object would be generated every time the store is queried.
   */
  private serialize(ns: TSNamespace, context: Context): NamespaceAndContext {
    return `${ns}${context}` as NamespaceAndContext;
  }

  /** Get a symbol VName for a given namespace and context, if it exists. */
  get(symbol: ts.Symbol, ns: TSNamespace, context: Context): VName|undefined {
    if (this.store.has(symbol)) {
      const nsCtx = this.serialize(ns, context);
      return this.store.get(symbol)!.get(nsCtx);
    }
    return undefined;
  }

  /**
   * Set a symbol VName for a given namespace and context. Throws if a VName
   * already exists.
   */
  set(symbol: ts.Symbol, ns: TSNamespace, context: Context, vname: VName) {
    let vnameMap = this.store.get(symbol);
    const nsCtx = this.serialize(ns, context);
    if (vnameMap) {
      if (vnameMap.has(nsCtx)) {
        throw new Error(`VName already set with signature ${
            vnameMap.get(nsCtx)!.signature}`);
      }
      vnameMap.set(nsCtx, vname);
    } else {
      this.store.set(symbol, new Map([[nsCtx, vname]]));
    }

    // Set the symbol VName for the given namespace and `Any` context, if it has
    // not already been set.
    const nsAny = this.serialize(ns, Context.Any);
    vnameMap = this.store.get(symbol)!;
    if (!vnameMap.has(nsAny)) {
      vnameMap.set(nsAny, vname);
    }
  }
}

/**
 * StandardIndexerContext provides the standard definition of information about
 * a TypeScript program and common methods used by the TypeScript indexer and
 * its plugins. See the IndexerContext interface definition for more details.
 */
class StandardIndexerContext implements IndexerHost {
  private offsetTables = new Map<string, utf8.OffsetTable>();

  /** A shorter name for the rootDir in the CompilerOptions. */
  private sourceRoot: string;

  /**
   * rootDirs is the list of rootDirs in the compiler options, sorted
   * longest first.  See this.moduleName().
   */
  private rootDirs: string[];

  /** symbolNames is a store of ts.Symbols to their assigned VNames. */
  private symbolNames = new SymbolVNameStore();

  /**
   * anonId increments for each anonymous block, to give them unique
   * signatures.
   */
  private anonId = 0;

  /**
   * anonNames maps nodes to the anonymous names assigned to them.
   */
  private anonNames = new Map<ts.Node, string>();

  private typeChecker: ts.TypeChecker;

  constructor(
    public readonly program: ts.Program,
    public readonly compilationUnit: CompilationUnit,
    public readonly options: IndexingOptions,
  ) {
    this.sourceRoot = this.program.getCompilerOptions().rootDir || process.cwd();
    let rootDirs = this.program.getCompilerOptions().rootDirs || [this.sourceRoot];
    rootDirs = rootDirs.map(d => d + '/');
    rootDirs.sort((a, b) => b.length - a.length);
    this.rootDirs = rootDirs;
    this.typeChecker = this.program.getTypeChecker();
  }

  getOffsetTable(path: string): Readonly<utf8.OffsetTable> {
    let table = this.offsetTables.get(path);
    if (!table) {
      const buf = (this.options.readFile || fs.readFileSync)(path);
      table = new utf8.OffsetTable(buf);
      this.offsetTables.set(path, table);
    }
    return table;
  }

  getSymbolAtLocation(node: ts.Node): ts.Symbol|undefined {
    // Practically any interesting node has a Symbol: variables, classes, functions.
    // Both named and anonymous have Symbols. We tie Symbols to Vnames so its
    // important to get Symbol object for as many nodes as possible. Unfortunately
    // Typescript doesn't provide good API for extracting Symbol from Nodes.
    // It is supported well for named nodes, probably logic being that if you can't
    // refer to a node then no need to have Symbol. But for Kythe we need to handle
    // anonymous nodes as well. So we do hacks here.
    // See similar bugs that haven't been resolved properly:
    // https://github.com/microsoft/TypeScript/issues/26511
    //
    // Open FR: https://github.com/microsoft/TypeScript/issues/55433
    let sym = this.typeChecker.getSymbolAtLocation(node);
    if (sym) return sym;
    // Check if it's named node.
    if ('name' in node) {
      sym = this.typeChecker.getSymbolAtLocation((node as ts.NamedDeclaration).name!);
      if (sym) return sym;
    }
    // Sad hack. Nodes have symbol property but it's not exposed in the API.
    // We could create our own Symbol instance to avoid depending on non-public API.
    // But it's not clear whether it will be less maintainance.
    return (node as any).symbol;
  }

  getSymbolAtLocationFollowingAliases(node: ts.Node): ts.Symbol|undefined {
    let sym = this.getSymbolAtLocation(node);
    while (sym && (sym.flags & ts.SymbolFlags.Alias) > 0) {
      // a hack to prevent following aliases in cases like:
      // import * as fooNamespace from './foo';
      // here fooNamespace is an alias for the 'foo' module.
      // We don't want to follow it so that users can easier usages
      // of fooNamespace in the file.
      const decl = sym.declarations?.[0];
      if (decl && ts.isNamespaceImport(decl)) {
        break;
      }

      sym = this.typeChecker.getAliasedSymbol(sym);
    }
    return sym;
  }

  /**
   * anonName assigns a freshly generated name to a Node.
   * It's used to give stable names to e.g. anonymous objects.
   */
  anonName(node: ts.Node): string {
    let name = this.anonNames.get(node);
    if (!name) {
      name = `anon${this.anonId++}`;
      this.anonNames.set(node, name);
    }
    return name;
  }

  /**
   * scopedSignature computes a scoped name for a ts.Node.
   * E.g. if you have a function `foo` containing a block containing a variable
   * `bar`, it might return a VName like
   *   signature: "foo.block0.bar""
   *   path: <appropriate path to module>
   */
  scopedSignature(startNode: ts.Node): VName {
    let moduleName: string|undefined;
    const parts: string[] = [];

    // Traverse the containing blocks upward, gathering names from nodes that
    // introduce scopes.
    for (let node: ts.Node|undefined = startNode,
                   lastNode: ts.Node|undefined = undefined;
         node != null; lastNode = node, node = node.parent) {
      // Nodes that are rvalues of a named initialization should not introduce a
      // new scope. For instance, in `const a = class A {}`, `A` should
      // contribute nothing to the scoped signature.
      if (node.parent && hasExpressionInitializer(node.parent) &&
          node.parent.name.kind === ts.SyntaxKind.Identifier) {
        continue;
      }

      switch (node.kind) {
        case ts.SyntaxKind.ExportAssignment:
          const exportDecl = node as ts.ExportAssignment;
          if (!exportDecl.isExportEquals) {
            // It's an "export default" statement.
            // This is semantically equivalent to exporting a variable
            // named 'default'.
            parts.push('default');
          } else {
            parts.push('export=');
          }
          break;
        case ts.SyntaxKind.ArrowFunction:
          // Arrow functions are anonymous, so generate a unique id.
          parts.push(`arrow${this.anonId++}`);
          break;
        case ts.SyntaxKind.FunctionExpression:
          // Function expressions look like
          //   (function() {})
          // which have no name but introduce an anonymous scope.
          parts.push(`func${this.anonId++}`);
          break;
        case ts.SyntaxKind.Block:
          // Blocks need their own scopes for contained variable declarations.
          if (node.parent &&
              (node.parent.kind === ts.SyntaxKind.FunctionDeclaration ||
               node.parent.kind === ts.SyntaxKind.MethodDeclaration ||
               node.parent.kind === ts.SyntaxKind.Constructor ||
               node.parent.kind === ts.SyntaxKind.ForStatement ||
               node.parent.kind === ts.SyntaxKind.ForInStatement ||
               node.parent.kind === ts.SyntaxKind.ForOfStatement)) {
            // A block that's an immediate child of the above node kinds
            // already has a scoped name generated by that parent.
            // (It would be fine to not handle this specially and just fall
            // through to the below code, but avoiding it here makes the names
            // simpler.)
            continue;
          }
          parts.push(`block${this.anonId++}`);
          break;
        case ts.SyntaxKind.ForStatement:
        case ts.SyntaxKind.ForInStatement:
        case ts.SyntaxKind.ForOfStatement:
          // Introduce a naming scope for all variables declared within the
          // statement, so that the two 'x's declared here get different names:
          //   for (const x in y) { ... }
          //   for (const x in y) { ... }
          parts.push(`for${this.anonId++}`);
          break;
        case ts.SyntaxKind.BindingElement:
        case ts.SyntaxKind.ClassDeclaration:
        case ts.SyntaxKind.ClassExpression:
        case ts.SyntaxKind.EnumDeclaration:
        case ts.SyntaxKind.EnumMember:
        case ts.SyntaxKind.FunctionDeclaration:
        case ts.SyntaxKind.InterfaceDeclaration:
        case ts.SyntaxKind.ImportEqualsDeclaration:
        case ts.SyntaxKind.ImportSpecifier:
        case ts.SyntaxKind.ExportSpecifier:
        case ts.SyntaxKind.MethodDeclaration:
        case ts.SyntaxKind.MethodSignature:
        case ts.SyntaxKind.NamespaceImport:
        case ts.SyntaxKind.ObjectLiteralExpression:
        case ts.SyntaxKind.Parameter:
        case ts.SyntaxKind.PropertyAccessExpression:
        case ts.SyntaxKind.PropertyAssignment:
        case ts.SyntaxKind.PropertyDeclaration:
        case ts.SyntaxKind.PropertySignature:
        case ts.SyntaxKind.TypeAliasDeclaration:
        case ts.SyntaxKind.TypeParameter:
        case ts.SyntaxKind.VariableDeclaration:
        case ts.SyntaxKind.GetAccessor:
        case ts.SyntaxKind.SetAccessor:
        case ts.SyntaxKind.ShorthandPropertyAssignment:
          const decl = node as ts.NamedDeclaration;
          if (decl.name) {
            switch (decl.name.kind) {
              case ts.SyntaxKind.Identifier:
              case ts.SyntaxKind.StringLiteral:
              case ts.SyntaxKind.NumericLiteral:
              case ts.SyntaxKind.ComputedPropertyName:
              case ts.SyntaxKind.NoSubstitutionTemplateLiteral:
                let part;
                if (ts.isComputedPropertyName(decl.name)) {
                  const sym = this.getSymbolAtLocationFollowingAliases(decl.name);
                  part = sym ? sym.name : this.anonName(decl.name);
                } else {
                  part = decl.name.text;
                }
                // Wrap literals in quotes, so that characters used in other
                // signatures do not interfere with the signature created by a
                // literal. For instance, a literal
                //   obj.prop
                // may interefere with the signature of `prop` on an object
                // `obj`. The literal receives a signature
                //   "obj.prop"
                // to avoid this.
                if (ts.isStringLiteral(decl.name)) {
                  part = `"${part}"`;
                }
                // Instance members of a class are scoped to the type of the
                // class.
                if (ts.isClassDeclaration(decl) && lastNode !== undefined &&
                    ts.isClassElement(lastNode) &&
                    !isStaticMember(lastNode, decl) &&
                    // special case constructor. We want it to no have
                    // #type modifier as constructor will have the same name
                    // as class.
                    !ts.isConstructorDeclaration(startNode)) {
                  part += '#type';
                }
                // Getters and setters semantically refer to the same entities
                // but are declared differently, so they are differentiated.
                if (ts.isGetAccessor(decl)) {
                  part += ':getter';
                } else if (ts.isSetAccessor(decl)) {
                  part += ':setter';
                }
                parts.push(part);
                break;
              default:
                // Skip adding an anonymous scope for variables declared in an
                // array or object binding pattern like `const [a] = [0]`.
                break;
            }
          } else {
            parts.push(this.anonName(node));
          }
          break;
        case ts.SyntaxKind.Constructor:
          // Class members declared with a shorthand in the constructor should
          // be scoped to the class, not the constructor.
          if (!ts.isParameterPropertyDeclaration(startNode, startNode.parent) &&
              startNode !== node) {
            parts.push('constructor');
          }
          break;
        case ts.SyntaxKind.ImportClause:
          // An import clause can have one of two forms:
          //   import foo from './bar';
          //   import {foo as far} from './bar';
          // In the first case the clause has a name "foo". In this case add the
          // name of the clause to the signature.
          // In the second case the clause has no explicit name. This
          // contributes nothing to the signature without risk of naming
          // conflicts because TS imports are essentially file-global lvalues.
          const importClause = node as ts.ImportClause;
          if (importClause.name) {
            parts.push(importClause.name.text);
          }
          break;
        case ts.SyntaxKind.ModuleDeclaration:
          const modDecl = node as ts.ModuleDeclaration;
          if (modDecl.name.kind === ts.SyntaxKind.StringLiteral) {
            // Syntax like:
            //   declare module 'foo/bar' {}
            // This is the syntax for defining symbols in another, named
            // module.
            moduleName = (modDecl.name as ts.StringLiteral).text;
          } else if (modDecl.name.kind === ts.SyntaxKind.Identifier) {
            // Syntax like:
            //   declare module foo {}
            // without quotes is just an obsolete way of saying 'namespace'.
            parts.push((modDecl.name as ts.Identifier).text);
          }
          break;
        case ts.SyntaxKind.SourceFile:
          // moduleName can already be set if the target was contained within
          // a "declare module 'foo/bar'" block (see the handling of
          // ModuleDeclaration).  Otherwise, the module name is derived from the
          // name of the current file.
          if (!moduleName) {
            moduleName = this.moduleName((node as ts.SourceFile).fileName);
          }
          break;
        case ts.SyntaxKind.JsxElement:
        case ts.SyntaxKind.JsxSelfClosingElement:
        case ts.SyntaxKind.JsxAttribute:
          // Given a unique anonymous name to all JSX nodes. This prevents
          // conflicts in cases where attributes would otherwise have the same
          // name, like `src` in
          //   <img src={a} />
          //   <img src={b} />
          parts.push(`jsx${this.anonId++}`);
          break;
        default:
          // Most nodes are children of other nodes that do not introduce a
          // new namespace, e.g. "return x;", so ignore all other parents
          // by default.
          // TODO: namespace {}, etc.

          // If the node is actually some subtype that has a 'name' attribute
          // and it's not empty it's likely this function should have handled
          // it. Dynamically probe for this case and warn if we missed one.
          if ((node as any).name != null) {
            todo(
                this.sourceRoot, node,
                `scopedSignature: ${ts.SyntaxKind[node.kind]} ` +
                    `has unused 'name' property`);
          }
      }
    }

    // The names were gathered from bottom to top, so reverse before joining.
    const signature = parts.reverse().join('.');
    return Object.assign(
        this.pathToVName(moduleName!), {signature, language: LANGUAGE});
  }

  /**
   * getSymbolName computes the VName of a ts.Symbol. A Context can be
   * optionally specified to help disambiguate nodes with multiple declarations.
   * See the documentation of Context for more information.
   */
  getSymbolName(
      sym: ts.Symbol, ns: TSNamespace, context: Context = Context.Any): VName
      |undefined {
    const stored = this.symbolNames.get(sym, ns, context);
    if (stored) return stored;

    if (!sym.declarations || sym.declarations.length < 1) {
      return undefined;
    }

    let declarations = sym.declarations;
    // Disambiguate symbols with multiple declarations using a context.
    if (sym.declarations.length > 1) {
      switch (context) {
        case Context.Getter:
          declarations = declarations.filter(ts.isGetAccessor);
          break;
        case Context.Setter:
          declarations = declarations.filter(ts.isSetAccessor);
          break;
        default:
          break;
      }
    }

    const decl = declarations[0];
    const vname = this.scopedSignature(decl);
    // The signature of a value is undecorated.
    // The signature of a type has the #type suffix.
    // The signature of a namespace has the #namespace suffix.
    if (ns === TSNamespace.TYPE) {
      vname.signature += '#type';
    } else if (ns === TSNamespace.NAMESPACE) {
      vname.signature += '#namespace';
    } else if (ns === TSNamespace.TYPE_MIGRATION) {
      vname.signature += '#mtype';
    }

    // Cache the VName for future lookups.
    this.symbolNames.set(sym, ns, context, vname);
    return vname;
  }

  /**
   * moduleName returns the ES6 module name of a path to a source file.
   * E.g. foo/bar.ts and foo/bar.d.ts both have the same module name,
   * 'foo/bar', and rootDirs (like bazel-bin/) are eliminated.
   * See README.md for a discussion of this.
   */
  moduleName(sourcePath: string): string {
    // Compute sourcePath as relative to one of the rootDirs.
    // This canonicalizes e.g. bazel-bin/foo to just foo.
    // Note that this.rootDirs is sorted longest first, so we'll use the
    // longest match.
    for (const rootDir of this.rootDirs) {
      if (sourcePath.startsWith(rootDir)) {
        sourcePath = path.relative(rootDir, sourcePath);
        break;
      }
    }
    return stripExtension(sourcePath);
  }

  /**
   * pathToVName returns the VName for a given file path.
   *
   * This function is used for 2 distinct cases that should be ideally separated
   * in 2 different functions. `path` can be one of two:
   * 1. Full path like 'bazel-out/genfiles/path/to/file.ts'.
   *    This path is used to build VNames for files and anchors.
   * 2. Module name like 'path/to/file'.
   *    This path is used to build VNames for semantic nodes.
   *
   * Only for full paths `pathVnames` contains an entry. For short paths (module
   * names) this function will defaults to calculating vname based on path
   * and compilation unit.
   */
  pathToVName(path: string): VName {
    const vname = this.compilationUnit.fileVNames.get(path);
    return {
      signature: '',
      language: '',
      corpus: vname && vname.corpus ? vname.corpus :
                                      this.compilationUnit.rootVName.corpus,
      root: vname && vname.corpus ? vname.root : this.compilationUnit.rootVName.root,
      path: vname && vname.path ? vname.path : path,
    };
  }
}

const RE_FIRST_NON_WS = /\S|$/;
const RE_NEWLINE = /\r?\n/;
const MAX_MS_TEXT_LENGTH = 1_000;
/**
 * Formats a MarkedSource text component by
 * - stripping the text to its first MAX_MS_TEXT_LENGTH characters
 * - trimming the text
 * - stripping the leading whitespace of each line in multi-line string by the
 *   shortest non-zero whitespace length. For example,
 *   [
 *       1,
 *       2,
 *     ]
 *   becomes
 *   [
 *     1,
 *     2,
 *   ]
 */
function fmtMarkedSource(s: string) {
  if (s.search(RE_FIRST_NON_WS) === s.length) {
    // String is all whitespace, keep as-is
    return s;
  }
  let isChopped = false;
  if (s.length > MAX_MS_TEXT_LENGTH) {
    // Trim left first to pick up more chars before chopping the string
    s = s.trimLeft();
    s = s.substring(0, MAX_MS_TEXT_LENGTH);
    isChopped = true;
  }
  s = s.replace(/\t/g, '    ');  // normalize tabs for display
  const lines = s.split(RE_NEWLINE);
  let shortestLeading = lines[lines.length - 1].search(RE_FIRST_NON_WS);
  for (let i = 1; i < lines.length - 1; ++i) {
    shortestLeading =
        Math.min(shortestLeading, lines[i].search(RE_FIRST_NON_WS));
  }
  for (let i = 1; i < lines.length; ++i) {
    lines[i] = lines[i].substring(shortestLeading);
  }
  s = lines.join('\n');
  if (isChopped) {
    s += '...';
  }
  return s;
}

function isNonNullableArray<T>(arr: Array<T>): arr is Array<NonNullable<T>> {
  return arr.findIndex(el => el === undefined || el === null) === -1;
}

/** Visitor manages the indexing process for a single TypeScript SourceFile. */
class Visitor {
  /** kFile is the VName for the 'file' node representing the source file. */
  kFile: VName;

  /** A shorter name for the rootDir in the CompilerOptions. */
  sourceRoot: string;

  typeChecker: ts.TypeChecker;

  /** influencers is a stack of influencer VNames. */
  private readonly influencers: Set<VName>[] = [];

  /** Cached anchor nodes. Signature is used as key. */
  private readonly anchors = new Map<string, VName>();

  constructor(
      private readonly host: IndexerHost,
      private file: ts.SourceFile,
  ) {
    this.sourceRoot =
        this.host.program.getCompilerOptions().rootDir || process.cwd();

    this.typeChecker = this.host.program.getTypeChecker();

    this.kFile = this.newFileVName(file.fileName);
  }

  /**
   * newFileVName returns a new VName for the given file path.
   */
  newFileVName(path: string): VName {
    return this.host.pathToVName(path);
  }

  /**
   * newVName returns a new VName with a given signature and path.
   */
  newVName(signature: string, path: string): VName {
    return Object.assign(
        this.newFileVName(path), {signature: signature, language: LANGUAGE});
  }

  /** newAnchor emits a new anchor entry that covers a TypeScript node. */
  newAnchor(node: ts.Node, start = node.getStart(), end = node.end, tag = ''):
      VName {
    const signature = `@${tag}${start}:${end}`;
    const cachedName = this.anchors.get(signature);
    if (cachedName != null) return cachedName;

    const name =
        Object.assign({...this.kFile}, {signature, language: LANGUAGE});
    this.emitNode(name, NodeKind.ANCHOR);
    const offsetTable = this.host.getOffsetTable(node.getSourceFile().fileName);
    this.emitFact(
        name, FactName.LOC_START, offsetTable.lookupUtf8(start).toString());
    this.emitFact(
        name, FactName.LOC_END, offsetTable.lookupUtf8(end).toString());
    this.anchors.set(signature, name);
    return name;
  }

  /** emitNode emits a new node entry, declaring the kind of a VName. */
  emitNode(source: VName, kind: NodeKind) {
    this.emitFact(source, FactName.NODE_KIND, kind);
  }

  /** emitSubkind emits a new fact entry, declaring the subkind of a VName. */
  emitSubkind(source: VName, subkind: Subkind) {
    this.emitFact(source, FactName.SUBKIND, subkind);
  }

  /** emitFact emits a new fact entry, tying an attribute to a VName. */
  emitFact(source: VName, name: FactName, value: string|Uint8Array) {
    this.host.options.emit({
      source,
      fact_name: name,
      fact_value: Buffer.from(value).toString('base64'),
    });
  }

  /** emitEdge emits a new edge entry, relating two VNames. */
  emitEdge(source: VName, kind: EdgeKind|OrdinalEdge, target: VName) {
    this.host.options.emit({
      source,
      edge_kind: kind,
      target,
      fact_name: '/',
    });
  }

  /** forAllInfluencers executes fn for each influencer in the active set. */
  forAllInfluencers(fn: (influencer: VName) => void) {
    if (this.influencers.length != 0) {
      this.influencers[this.influencers.length - 1].forEach(fn);
    }
  }

  /** addInfluencer adds influencer to the active set. */
  addInfluencer(influencer: VName) {
    if (this.influencers.length != 0) {
      this.influencers[this.influencers.length - 1].add(influencer);
    }
  }

  /** popInfluencers pops the active influencer set. */
  popInfluencers() {
    if (this.influencers.length != 0) {
      this.influencers.pop();
    }
  }

  /** pushInfluencers pushes a new active influencer set. */
  pushInfluencers() {
    this.influencers.push(new Set<VName>());
  }

  visitTypeParameters(
      parent: VName|undefined,
      params: ReadonlyArray<ts.TypeParameterDeclaration>) {
    for (var ordinal = 0; ordinal < params.length; ++ordinal) {
      const param = params[ordinal];
      const sym = this.host.getSymbolAtLocationFollowingAliases(param.name);
      if (!sym) {
        todo(
            this.sourceRoot, param,
            `type param ${param.getText()} has no symbol`);
        return;
      }
      const kTVar = this.host.getSymbolName(sym, TSNamespace.TYPE_MIGRATION);
      if (kTVar && parent) {
        this.emitNode(kTVar, NodeKind.TVAR);
        this.emitEdge(
            this.newAnchor(
                param.name, param.name.getStart(), param.name.end, 'M'),
            EdgeKind.DEFINES_BINDING, kTVar);
        this.emitEdge(parent, makeOrdinalEdge(EdgeKind.TPARAM, ordinal), kTVar);
        // ...<T extends A>
        if (param.constraint) {
          var superType = this.visitType(param.constraint);
          if (superType)
            this.emitEdge(kTVar, EdgeKind.BOUNDED_UPPER, superType);
        }
        // ...<T = A>
        if (param.default) this.visitType(param.default);
      }
    }
  }

  /**
   * Adds influence edges for return statements.
   */
  visitReturnStatement(node: ts.ReturnStatement) {
    this.pushInfluencers();
    ts.forEachChild(node, n => {
      this.visit(n);
    });
    const containingFunction = this.getContainingFunctionNode(node);
    if (!ts.isSourceFile(containingFunction)) {
      const containingVName =
          this.getSymbolAndVNameForFunctionDeclaration(containingFunction)
              .vname;
      if (containingVName) {
        this.forAllInfluencers(influencer => {
          this.emitEdge(influencer, EdgeKind.INFLUENCES, containingVName);
        });
      }
      // Handle case like
      // "return {name: 'Alice'};"
      // where return type of the function is a named type, e.g. Person.
      // This will connect 'name' property of the object literal to the
      // Person.name property.
      if (node.expression && ts.isObjectLiteralExpression(node.expression)) {
        this.connectObjectLiteralToType(
            node.expression, containingFunction.type);
      }
    }
    this.popInfluencers();
  }

  getCallAnchor(callee:any) {
    if (!this.host.options.emitRefCallOverIdentifier) {
      return undefined;
    }
    for (;;) {
      if (ts.isIdentifier(callee)) {
        return this.newAnchor(callee);
      }
      if (ts.isPropertyAccessExpression(callee)) {
        callee = callee.name;
        continue;
      }
      if (ts.isNewExpression(callee)) {
        callee = callee.expression;
        continue;
      }
      return undefined;
    }
  }

  /**
   * Emits `ref/call` edges required for call graph:
   * https://kythe.io/docs/schema/callgraph.html
   */
  visitCallOrNewExpression(node: ts.CallExpression|ts.NewExpression) {
    ts.forEachChild(node, n => {
      this.visit(n);
    });

    // Special case dynamic imports as they are represendted as CallExpressions.
    // We don't want to emit ref/call as we don't have anything to point it at:
    // there is no import() function
    if (ts.isCallExpression(node) && node.expression.kind === ts.SyntaxKind.ImportKeyword) {
      this.visitDynamicImportCall(node);
      return;
    }
    // Handle case of immediately-invoked functions like:
    // (() => do stuff... )();
    let expression: ts.Node = node.expression;
    while (ts.isParenthesizedExpression(expression)) {
      expression = expression.expression;
    }
    const symbol = this.host.getSymbolAtLocationFollowingAliases(expression);
    if (!symbol) {
      return;
    }
    const name = this.host.getSymbolName(symbol, TSNamespace.VALUE);
    if (!name) {
      return;
    }
    const callAnchor = this.getCallAnchor(node.expression) ?? this.newAnchor(node);
    this.emitEdge(callAnchor, EdgeKind.REF_CALL, name);

    // Each call should have a childof edge to its containing function
    // scope.
    const containingFunction = this.getContainingFunctionNode(node);
    let containingVName: VName|undefined;
    if (ts.isSourceFile(containingFunction)) {
      containingVName = this.getSyntheticFileInitVName();
    } else {
      containingVName =
          this.getSymbolAndVNameForFunctionDeclaration(containingFunction)
              .vname;
    }
    if (containingVName) {
      this.emitEdge(callAnchor, EdgeKind.CHILD_OF, containingVName);
    }
    // Handle function/constructor calls like `doSomething({name: 'Alice'})`
    // where type of the argument is, for example, an interface Person. This
    // will add ref from object literal 'name' property to Person.name field.
    if (node.arguments != null) {
      const signature = this.typeChecker.getResolvedSignature(node);
      if (signature == null) return;
      for (let i = 0; i < node.arguments.length; i++) {
        const argument = node.arguments[i];
        // get parameter of the function/constructor signature.
        const signParameter = signature.parameters[i]?.valueDeclaration;
        if (ts.isObjectLiteralExpression(argument) && signParameter &&
            ts.isParameter(signParameter)) {
          this.connectObjectLiteralToType(argument, signParameter.type);
        }
      }
    }
  }

  /**
   * visitHeritage visits the X found in an 'extends X' or 'implements X'.
   *
   * These are subtle in an interesting way.  When you have
   *   interface X extends Y {}
   * that is referring to the *type* Y (because interfaces are types, not
   * values).  But it's also legal to write
   *   class X extends (class Z { ... }) {}
   * where the thing in the extends clause is itself an expression, and the
   * existing logic for visiting a class expression already handles modelling
   * the class as both a type and a value.
   *
   * The full set of possible combinations is:
   * - class extends => value
   * - interface extends => type
   * - class implements => type
   * - interface implements => illegal
   */
  visitHeritage(
      classOrInterface: VName|undefined,
      heritageClauses: ReadonlyArray<ts.HeritageClause>) {
    for (const heritage of heritageClauses) {
      if (heritage.token === ts.SyntaxKind.ExtendsKeyword && heritage.parent &&
          heritage.parent.kind !== ts.SyntaxKind.InterfaceDeclaration) {
        this.visit(heritage);
      } else {
        this.visitType(heritage);
      }
      // Add extends edges.
      if (classOrInterface == null) {
        // classOrInterface is null for anonymous classes.
        // But anonymous classes can implement and extends other
        // classes and interfaces. So currently this edge
        // is missing. Once we have nodes for anonymous classes -
        // we can add this missing edges.
        continue;
      }
      for (const baseType of heritage.types) {
        const type = this.typeChecker.getTypeAtLocation(baseType);
        if (!type || !type.symbol) {
          continue;
        }
        const vname = this.host.getSymbolName(type.symbol, TSNamespace.TYPE);
        if (vname) {
          this.emitEdge(classOrInterface, EdgeKind.EXTENDS, vname);
        }
      }
    }
  }

  visitInterfaceDeclaration(decl: ts.InterfaceDeclaration) {
    const sym = this.host.getSymbolAtLocationFollowingAliases(decl.name);
    if (!sym) {
      todo(
          this.sourceRoot, decl.name,
          `interface ${decl.name.getText()} has no symbol`);
      return;
    }
    const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
    if (kType) {
      this.emitNode(kType, NodeKind.INTERFACE);
      this.emitEdge(this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, kType);
      this.visitJSDoc(decl, kType);
      this.emitMarkedSourceForClasslikeDeclaration(decl, kType);
    }

    if (decl.typeParameters)
      this.visitTypeParameters(kType, decl.typeParameters);
    if (decl.heritageClauses) this.visitHeritage(kType, decl.heritageClauses);
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  visitTypeAliasDeclaration(decl: ts.TypeAliasDeclaration) {
    const sym = this.host.getSymbolAtLocationFollowingAliases(decl.name);
    if (!sym) {
      todo(
          this.sourceRoot, decl.name,
          `type alias ${decl.name.getText()} has no symbol`);
      return;
    }
    const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
    if (!kType) return;
    this.emitNode(kType, NodeKind.TALIAS);
    this.emitEdge(this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, kType);
    this.visitJSDoc(decl, kType);

    if (decl.typeParameters)
      this.visitTypeParameters(kType, decl.typeParameters);
    const kAlias = this.visitType(decl.type);
    // Emit an "aliases" edge if the aliased type has a singular VName.
    if (kAlias) {
      this.emitEdge(kType, EdgeKind.ALIASES, kAlias);
    }
  }

  /**
   * visitType is the main dispatch for visiting type nodes.
   * It's separate from visit() because bare ts.Identifiers within a normal
   * expression are values (handled by visit) but bare ts.Identifiers within
   * a type are types (handled here).
   *
   * @return the VName of the type, if available.  (For more complex types,
   *    e.g. Array<string>, we might not have a VName for the specific type.)
   */
  visitType(node: ts.Node): VName|undefined {
    switch (node.kind) {
      case ts.SyntaxKind.TypeReference:
        const ref = node as ts.TypeReferenceNode;
        const kType = this.visitType((node as ts.TypeReferenceNode).typeName);

        // Return no VName for types with type arguments because their VName is
        // not qualified. E.g.
        //   Array<string>
        //   Array<number>
        // have the same VName signature "Array"
        if (ref.typeArguments) {
          ref.typeArguments.forEach(type => this.visitType(type));
          return;
        }
        return kType;
      case ts.SyntaxKind.Identifier:
        const sym = this.host.getSymbolAtLocationFollowingAliases(node);
        if (!sym) {
          todo(this.sourceRoot, node, `type ${node.getText()} has no symbol`);
          return;
        }
        const name = this.host.getSymbolName(sym, TSNamespace.TYPE);
        if (name) {
          this.emitEdge(this.newAnchor(node), EdgeKind.REF, name);
        }
        const mname = this.host.getSymbolName(sym, TSNamespace.TYPE_MIGRATION);
        if (mname) {
          this.emitEdge(
              this.newAnchor(node, node.getStart(), node.end, 'M'),
              EdgeKind.REF, mname);
        }
        return name;
      case ts.SyntaxKind.TypeReference:
        const typeRef = node as ts.TypeReferenceNode;
        if (!typeRef.typeArguments) {
          // If it's an direct type reference, e.g. SomeInterface
          // as opposed to SomeInterface<T>, then the VName for the type
          // reference is just the inner type name.
          return this.visitType(typeRef.typeName);
        }
        // Otherwise, leave it to the default handling.
        break;
      case ts.SyntaxKind.TypeQuery:
        // This is a 'typeof' expression, which takes a value as its argument,
        // so use visit() instead of visitType().
        const typeQuery = node as ts.TypeQueryNode;
        this.visit(typeQuery.exprName);
        return;  // Avoid default recursion.
    }

    // Default recursion, but using visitType(), not visit().
    ts.forEachChild(node, n => {
      this.visitType(n);
    });
    // Because we don't know the specific thing we visited, give the caller
    // back no name.
    return undefined;
  }

  /**
   * getPathFromModule gets the "module path" from the module import
   * symbol referencing a module system path to reference to a module.
   *
   * E.g. from
   *   import ... from './foo';
   * getPathFromModule(the './foo' node) might return a string like
   * 'path/to/project/foo'.  See this.moduleName().
   */
  getModulePathFromModuleReference(sym: ts.Symbol): string|undefined {
    const sf = sym.valueDeclaration;
    // If the module is not a source file, it does not have a unique file path.
    // This can happen in cases of importing local modules, like
    //   declare namespace Foo {}
    //   import foo = Foo;
    if (!sf || !ts.isSourceFile(sf)) return undefined;
    return this.host.moduleName(sf.fileName);
  }

  /**
   * Returns the location of a text in the source code of a node.
   */
  getTextSpan(node: ts.Node, text: string): {start: number, end: number} {
    const ofs = node.getText().indexOf(text);
    if (ofs < 0) throw new Error(`${text} not found in ${node.getText()}`);
    const start = node.getStart() + ofs;
    const end = start + text.length;
    return {start, end};
  }

  /**
   * Returns the symbol of a class constructor if it exists, otherwise nothing.
   */
  getCtorSymbol(klass: ts.ClassDeclaration): ts.Symbol|undefined {
    if (klass.name) {
      const sym = this.host.getSymbolAtLocationFollowingAliases(klass.name);
      if (sym && sym.members) {
        return sym.members.get(ts.InternalSymbolName.Constructor);
      }
    }
    return undefined;
  }

  getSymbolAndVNameForFunctionDeclaration(node: ts.FunctionLikeDeclaration):
      {sym?: ts.Symbol, vname?: VName} {
    let context: Context|undefined = undefined;
    if (ts.isGetAccessor(node)) {
      context = Context.Getter;
    } else if (ts.isSetAccessor(node)) {
      context = Context.Setter;
    }
    const sym = this.host.getSymbolAtLocationFollowingAliases(node);
    if (!sym) {
      return {};
    }
    const vname = this.host.getSymbolName(sym, TSNamespace.VALUE, context);
    return {sym, vname};
  }

  /**
   * Given a node finds a function node that contains given node and returns it.
   * If the node is not inside a function - returns SourceFile node. Examples:
   *
   * function foo() {
   *   var b = 123;
   * }
   * var c = 567;
   *
   * For 'b' node this function will return 'foo' node.
   * For 'c' node this function will return SourceFile node.
   */
  getContainingFunctionNode(node: ts.Node): ts.FunctionLikeDeclaration
      |ts.SourceFile {
    node = node.parent;
    for (; node.kind !== ts.SyntaxKind.SourceFile; node = node.parent) {
      const kind = node.kind;
      if (kind === ts.SyntaxKind.FunctionDeclaration ||
          kind === ts.SyntaxKind.FunctionExpression ||
          kind === ts.SyntaxKind.ArrowFunction ||
          kind === ts.SyntaxKind.MethodDeclaration ||
          kind === ts.SyntaxKind.Constructor ||
          kind === ts.SyntaxKind.GetAccessor ||
          kind === ts.SyntaxKind.SetAccessor ||
          kind === ts.SyntaxKind.MethodSignature) {
        return node as ts.FunctionLikeDeclaration;
      }
    }
    return node as ts.SourceFile;
  }

  getSyntheticFileInitVName(): VName {
    return this.newVName(
        'fileInit:synthetic', this.host.moduleName(this.file.fileName));
  }

  /**
   * visitImport handles a single entry in an import statement, e.g.
   * "bar" in code like
   *   import {foo, bar} from 'baz';
   * See visitImportDeclaration for the code handling the entire statement.
   *
   * @param name TypeScript import declaration node
   * @param bindingAnchor anchor that "defines/binding" the local import
   *     definition. If the bindingAnchor is null then the local definition
   *     won't be created and refs to the local definition will be reassigned
   *     to the remote definition.
   * @param refAnchor anchor that "ref" the import's remote declaration
   */
  visitImport(
      name: ts.Node, bindingAnchor: Readonly<VName>|null,
      refAnchor: Readonly<VName>) {
    // An import both aliases a symbol from another module
    // (call it the remote symbol) and it defines a local symbol.
    //
    // Those two symbols often have the same name, with statements like:
    //   import {foo} from 'bar';
    // But they can be different, in e.g.
    //   import {foo as baz} from 'bar';
    // Which maps the remote symbol named 'foo' to a local named 'baz'.
    //
    // In all cases TypeScript maintains two different ts.Symbol objects,
    // one for the local and one for the remote.  In principle for the
    // simple import statement
    //   import {foo} from 'bar';
    // "foo" should both:
    //   - "ref/imports" the remote symbol
    //   - "defines/binding" the local symbol

    const localSym = this.host.getSymbolAtLocation(name);
    if (!localSym) {
      throw new Error(`TODO: local name ${name} has no symbol`);
    }
    const remoteSym = this.typeChecker.getAliasedSymbol(localSym);

    // The imported symbol can refer to a type, a value, or both. Attempt to
    // define local imports and reference remote definitions in both cases.
    if (remoteSym.flags & ts.SymbolFlags.Value) {
      const kRemoteValue =
          this.host.getSymbolName(remoteSym, TSNamespace.VALUE);
      const kLocalValue = this.host.getSymbolName(localSym, TSNamespace.VALUE);
      if (!kRemoteValue || !kLocalValue) return;

      if (bindingAnchor) {
        // The local import value is a "variable" with an "import" subkind, and
        // aliases its remote definition.
        this.emitNode(kLocalValue, NodeKind.VARIABLE);
        this.emitFact(kLocalValue, FactName.SUBKIND, Subkind.IMPORT);
        this.emitEdge(kLocalValue, EdgeKind.ALIASES, kRemoteValue);
        this.emitEdge(bindingAnchor, EdgeKind.DEFINES_BINDING, kLocalValue);
      }
      // Emit edges from the referencing anchor to the import's remote definition.
      this.emitEdge(refAnchor, EdgeKind.REF_IMPORTS, kRemoteValue);
    }
    if (remoteSym.flags & ts.SymbolFlags.Type) {
      const kRemoteType = this.host.getSymbolName(remoteSym, TSNamespace.TYPE);
      const kLocalType = this.host.getSymbolName(localSym, TSNamespace.TYPE);
      if (!kRemoteType || !kLocalType) return;

      if (bindingAnchor) {
        // The local import value is a "talias" (type alias) with an "import"
        // subkind, and aliases its remote definition.
        this.emitNode(kLocalType, NodeKind.TALIAS);
        this.emitFact(kLocalType, FactName.SUBKIND, Subkind.IMPORT);
        this.emitEdge(kLocalType, EdgeKind.ALIASES, kRemoteType);
        this.emitEdge(bindingAnchor, EdgeKind.DEFINES_BINDING, kLocalType);
      }
      // Emit edges from the referencing anchor to the import's remote definition.
      this.emitEdge(refAnchor, EdgeKind.REF_IMPORTS, kRemoteType);
    }
  }

  /**
   * Emits `ref/import` edge from a module name to its definition.
   * It is used by static and dynamic imports:
   *
   * import {Bar} from './foo'; // static
   *
   * const {Bar} = await import('./foo'); // dynamic
   *
   * @param moduleRef Module reference. E.g. './foo' from the examples above.
   * @returns true if corresponding module was found and an edge was emitted.
   */
  emitRefToImportedModule(moduleRef: ts.Node): boolean {
    const moduleSym = this.host.getSymbolAtLocation(moduleRef);
    if (!moduleSym) {
      // This can occur when the module failed to resolve to anything.
      // See testdata/import_missing.ts for more on how that could happen.
      return false;
    }
    const modulePath = this.getModulePathFromModuleReference(moduleSym);
    if (modulePath) {
      const kModule = this.newVName('module', modulePath);
      this.emitEdge(this.newAnchor(moduleRef), EdgeKind.REF_IMPORTS, kModule);
    } else {
      // Check if module being imported is declared via `declare module`
      // and if so - output ref to that statement.
      const decl = moduleSym.valueDeclaration;
      if (decl && ts.isModuleDeclaration(decl)) {
        const kModule =
            this.host.getSymbolName(moduleSym, TSNamespace.NAMESPACE);
        if (!kModule) return false;
        this.emitEdge(this.newAnchor(moduleRef), EdgeKind.REF_IMPORTS, kModule);
      }
    }
    return true;
  }

  /** visitImportDeclaration handles the various forms of "import ...". */
  visitImportDeclaration(decl: ts.ImportDeclaration|
                         ts.ImportEqualsDeclaration) {
    // All varieties of import statements reference a module on the right,
    // so start by linking that.
    let moduleRef;
    if (ts.isImportDeclaration(decl)) {
      // This is a regular import declaration
      //     import ... from ...;
      // where the module name is moduleSpecifier.
      moduleRef = decl.moduleSpecifier;
    } else {
      // This is an import equals declaration, which has two cases:
      //     import foo = require('./bar');
      //     import foo = M.bar;
      // In the first case the moduleReference is an ExternalModuleReference
      // whose module name is the expression inside the `require` call.
      // In the second case the moduleReference is the module name.
      moduleRef = ts.isExternalModuleReference(decl.moduleReference) ?
          decl.moduleReference.expression :
          decl.moduleReference;
    }
    const moduleFound = this.emitRefToImportedModule(moduleRef);
    if (!moduleFound) {
      return;
    }

    // TODO(#4021): See discussion.
    // Pending changes, an anchor in a Code Search UI cannot currently be
    // displayed as a node definition and as referencing other nodes.  Instead,
    // for non-renamed imports the local node definition is placed on the
    // "import" text:
    //   //- @foo ref BarFoo
    //   //- @import defines/binding LocalFoo
    //   //- LocalFoo aliases BarFoo
    //   import {foo} from 'bar';
    // For renamed imports the definition and references are separated as
    // expected:
    //   //- @foo ref BarFoo
    //   //- @baz defines/binding LocalBaz
    //   //- @baz aliases BarFoo
    //   import {foo as baz} from 'bar';
    //
    // Create an anchor for the import text.
    const importTextSpan = this.getTextSpan(decl, 'import');
    const importTextAnchor =
        this.newAnchor(decl, importTextSpan.start, importTextSpan.end);

    if (ts.isImportEqualsDeclaration(decl)) {
      // This is an equals import, e.g.:
      //   import foo = require('./bar');
      //
      // TODO(#4021): Bind the local definition and reference the remote
      // definition on the import name.
      const refAnchor = this.newAnchor(decl.name);
      this.visitImport(decl.name, /* bindingAnchor= */ null, refAnchor);
      return;
    }

    if (!decl.importClause) {
      // This is a side-effecting import that doesn't declare anything, e.g.:
      //   import 'foo';
      return;
    }
    const clause = decl.importClause;

    if (clause.name) {
      // This is a default import, e.g.:
      //   import foo from './bar';
      //
      // TODO(#4021): Bind the local definition and reference the remote
      // definition on the import name.
      const refAnchor = this.newAnchor(clause.name);
      this.visitImport(clause.name, /* bindingAnchor= */ null, refAnchor);
      return;
    }

    if (!clause.namedBindings) {
      // TODO: I believe clause.name or clause.namedBindings are always present,
      // which means this check is not necessary, but the types don't show that.
      throw new Error(`import declaration ${decl.getText()} has no bindings`);
    }
    switch (clause.namedBindings.kind) {
      case ts.SyntaxKind.NamespaceImport:
        // This is a namespace import, e.g.:
        //   import * as foo from 'foo';
        const name = clause.namedBindings.name;
        const sym = this.host.getSymbolAtLocation(name);
        if (!sym) {
          todo(
              this.sourceRoot, clause,
              `namespace import ${clause.getText()} has no symbol`);
          return;
        }
        const kModuleObject = this.host.getSymbolName(sym, TSNamespace.VALUE);
        if (!kModuleObject) return;
        this.emitNode(kModuleObject, NodeKind.VARIABLE);
        this.emitEdge(
            this.newAnchor(name), EdgeKind.DEFINES_BINDING, kModuleObject);
        break;
      case ts.SyntaxKind.NamedImports:
        // This is named imports, e.g.:
        //   import {bar, baz} from 'foo';
        const imports = clause.namedBindings.elements;
        for (const imp of imports) {
          // If the named import has a property name, e.g. `bar` in
          //   import {bar as baz} from 'foo';
          // bind the local definition on the import name "baz" and reference
          // the remote definition on the property name "bar". Otherwise, bind
          // the local definition on "import" and reference the remote
          // definition on the import name.
          //
          // TODO(#4021): Unify binding and reference anchors.
          let bindingAnchor, refAnchor;
          if (imp.propertyName) {
            bindingAnchor = this.newAnchor(imp.name);
            refAnchor = this.newAnchor(imp.propertyName);
          } else {
            refAnchor = this.newAnchor(imp.name);
            bindingAnchor = null;
          }
          this.visitImport(imp.name, bindingAnchor, refAnchor);
        }
        break;
    }
  }

  /**
   * Handles dynamic imports like:
   *
   * const {SomeSym} = await import('./another/file');
   *
   * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/import
   *
   * @param node import() statement. TS represents dynamic imports as CallExpressions.
   */
  visitDynamicImportCall(node: ts.CallExpression) {
    const moduleRef = node.arguments[0];
    this.emitRefToImportedModule(moduleRef);
  }

  /**
   * When a file imports another file, with syntax like
   *   import * as x from 'some/path';
   * we wants 'some/path' to refer to a VName that just means "the entire
   * file".  It doesn't refer to any text in particular, so we just mark
   * the first letter in the file as the anchor for this.
   */
  emitModuleAnchor(sf: ts.SourceFile) {
    const kMod =
        this.newVName('module', this.host.moduleName(this.file.fileName));
    this.emitFact(kMod, FactName.NODE_KIND, NodeKind.RECORD);
    this.emitEdge(this.kFile, EdgeKind.CHILD_OF, kMod);

    // Emit the anchor, bound to the beginning of the file.
    const anchor = this.newAnchor(this.file, 0, 0);
    this.emitEdge(anchor, EdgeKind.DEFINES_IMPLICIT, kMod);
  }

  /**
   * Emits an implicit property for a getter or setter.
   * For instance, a getter/setter `foo` in class `A` will emit an implicit
   * property on that class with signature `A.foo`, and create "property/reads"
   * and "property/writes" from the getters/setters to the implicit property.
   */
  emitImplicitProperty(
      decl: ts.GetAccessorDeclaration|ts.SetAccessorDeclaration, anchor: VName,
      funcVName: VName) {
    // Remove trailing ":getter"/":setter" suffix
    const propSignature = funcVName.signature.split(':').slice(0, -1).join(':');
    const implicitProp = {...funcVName, signature: propSignature};

    this.emitNode(implicitProp, NodeKind.VARIABLE);
    this.emitSubkind(implicitProp, Subkind.IMPLICIT);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, implicitProp);

    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym || !sym.declarations) {
      throw new Error('Getter/setter declaration has no symbols.');
    }

    if (sym.declarations.find(ts.isGetAccessor)) {
      // Emit a "property/reads" edge between the getter and the property
      const getter =
          this.host.getSymbolName(sym, TSNamespace.VALUE, Context.Getter);
      if (!getter) return;
      this.emitEdge(getter, EdgeKind.PROPERTY_READS, implicitProp);
    }
    if (sym.declarations.find(ts.isSetAccessor)) {
      // Emit a "property/writes" edge between the setter and the property
      const setter =
          this.host.getSymbolName(sym, TSNamespace.VALUE, Context.Setter);
      if (!setter) return;
      this.emitEdge(setter, EdgeKind.PROPERTY_WRITES, implicitProp);
    }
  }

  /**
   * Handles code like:
   *   export default ...;
   *   export = ...;
   */
  visitExportAssignment(assign: ts.ExportAssignment) {
    if (assign.isExportEquals) {
      const span = this.getTextSpan(assign, 'export =');
      const anchor = this.newAnchor(assign, span.start, span.end);
      this.emitEdge(
          anchor, EdgeKind.DEFINES_BINDING, this.host.scopedSignature(assign));
    } else {
      // export default <expr>;
      // is the same as exporting the expression under the symbol named
      // "default".  But we don't have a nice name to link the symbol to!
      // So instead we link the keyword "default" itself to the VName.
      // The TypeScript AST does not expose the location of the 'default'
      // keyword so we just find it in the source text to link it.
      const span = this.getTextSpan(assign, 'default');
      const anchor = this.newAnchor(assign, span.start, span.end);
      this.emitEdge(
          anchor, EdgeKind.DEFINES_BINDING, this.host.scopedSignature(assign));
    }
  }

  /**
   * Handles code that explicitly exports a symbol, like:
   *   export {Foo} from './bar';
   *
   * Note that export can also be a modifier on a declaration, like:
   *   export class Foo {}
   * and that case is handled as part of the ordinary declaration handling.
   */
  visitExportDeclaration(decl: ts.ExportDeclaration) {
    if (decl.exportClause && ts.isNamedExports(decl.exportClause)) {
      for (const exp of decl.exportClause.elements) {
        const localSym = this.host.getSymbolAtLocation(exp.name);
        if (!localSym) {
          console.error(`TODO: export ${exp.name} has no symbol`);
          continue;
        }
        const remoteSym = this.typeChecker.getAliasedSymbol(localSym);
        const anchor = this.newAnchor(exp.name);
        // Aliased export; propertyName is the 'as <...>' bit.
        const propertyAnchor =
            exp.propertyName ? this.newAnchor(exp.propertyName) : null;
        // Symbol is a value.
        if (remoteSym.flags & ts.SymbolFlags.Value) {
          const kExport = this.host.getSymbolName(remoteSym, TSNamespace.VALUE);
          if (kExport) {
            this.emitEdge(anchor, EdgeKind.REF, kExport);
            if (propertyAnchor) {
              this.emitEdge(propertyAnchor, EdgeKind.REF, kExport);
            }
          }
        }
        // Symbol is a type.
        if (remoteSym.flags & ts.SymbolFlags.Type) {
          const kExport = this.host.getSymbolName(remoteSym, TSNamespace.TYPE);
          if (kExport) {
            this.emitEdge(anchor, EdgeKind.REF, kExport);
            if (propertyAnchor) {
              this.emitEdge(propertyAnchor, EdgeKind.REF, kExport);
            }
          }
        }
      }
    }
    if (decl.moduleSpecifier) {
      const moduleSym = this.host.getSymbolAtLocation(decl.moduleSpecifier);
      if (moduleSym) {
        const moduleName = this.getModulePathFromModuleReference(moduleSym);
        if (moduleName) {
          const kModule = this.newVName('module', moduleName);
          this.emitEdge(
              this.newAnchor(decl.moduleSpecifier), EdgeKind.REF_IMPORTS,
              kModule);
        }
      }
    }
  }

  visitVariableStatement(stmt: ts.VariableStatement) {
    // A VariableStatement contains potentially multiple variable declarations,
    // as in:
    //   var x = 3, y = 4;
    // In the (common) case where there's a single variable declared, we look
    // for documentation for that variable above the entire statement.
    if (stmt.declarationList.declarations.length === 1) {
      const vname =
          this.visitVariableDeclaration(stmt.declarationList.declarations[0]);
      if (vname) this.visitJSDoc(stmt, vname);
      return;
    }

    // Otherwise, use default recursion over the statement.
    ts.forEachChild(stmt, n => this.visit(n));
  }

  /**
   * Note: visitVariableDeclaration is also used for class properties;
   * the decl parameter is the union of the attributes of the two types.
   * @return the generated VName for the declaration, if any.
   */
  visitVariableDeclaration(decl: {
    name: ts.BindingName|ts.PropertyName,
    type?: ts.TypeNode,
    initializer?: ts.Expression, kind: ts.SyntaxKind,
  }&ts.Node): VName|undefined {
    let vname: VName|undefined;
    switch (decl.name.kind) {
      case ts.SyntaxKind.Identifier:
      case ts.SyntaxKind.StringLiteral:
      case ts.SyntaxKind.NumericLiteral:
        const sym = this.host.getSymbolAtLocation(decl.name);
        if (!sym) {
          todo(
              this.sourceRoot, decl.name,
              `declaration ${decl.name.getText()} has no symbol`);
          return undefined;
        }
        vname = this.host.getSymbolName(sym, TSNamespace.VALUE);
        if (vname) {
          this.emitNode(vname, NodeKind.VARIABLE);
          this.emitEdge(
              this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, vname);
        }

        decl.name.forEachChild(child => this.visit(child));
        break;
      case ts.SyntaxKind.ObjectBindingPattern:
      case ts.SyntaxKind.ArrayBindingPattern:
        for (const element of (decl.name as ts.BindingPattern).elements) {
          this.visit(element);
        }
        // Handle case like `const {name} = p;` where p is or type Person.
        // This will add ref from the `name` variable to Person.name.
        if (ts.isObjectBindingPattern(decl.name)) {
          this.connectObjectBindingPatternToType(decl.name);
        }
        break;
      case ts.SyntaxKind.ComputedPropertyName:
        decl.name.forEachChild(child => this.visit(child));
        break;
      default:
        break;
    }

    if (decl.type) this.visitType(decl.type);
    if (decl.initializer) this.visit(decl.initializer);
    if (decl.type && decl.initializer &&
        ts.isObjectLiteralExpression(decl.initializer)) {
      this.connectObjectLiteralToType(decl.initializer, decl.type);
    }
    if (!vname) {
      return undefined;
    }

    if (ts.isVariableDeclaration(decl) || ts.isPropertyAssignment(decl) ||
        ts.isPropertyDeclaration(decl) || ts.isBindingElement(decl) ||
        ts.isShorthandPropertyAssignment(decl) ||
        ts.isPropertySignature(decl) || ts.isJsxAttribute(decl)) {
      this.emitMarkedSourceForVariable(decl, vname);
    } else {
      todo(this.sourceRoot, decl, 'Emit variable delaration code');
    }

    if (ts.isPropertyDeclaration(decl)) {
      const declNode = decl as ts.PropertyDeclaration;
      if (isStaticMember(declNode, declNode.parent)) {
        this.emitFact(vname, FactName.TAG_STATIC, '');
      }
    }
    if (ts.isPropertySignature(decl) ||
        ts.isPropertyDeclaration(decl) ||
        ts.isPropertyAssignment(decl) ||
        ts.isShorthandPropertyAssignment(decl)) {
      this.emitSubkind(vname, Subkind.FIELD);
      this.emitChildofEdge(vname, decl.parent);
    }
    if (ts.isShorthandPropertyAssignment(decl)) {
      const origSym = this.typeChecker.getShorthandAssignmentValueSymbol(decl);
      if (origSym) {
        const origVName = this.host.getSymbolName(origSym, TSNamespace.VALUE);
        if (origVName) {
          this.emitEdge(this.newAnchor(decl.name), EdgeKind.REF_ID, origVName);
        }
      }
    }
    return vname;
  }

  getIdentifierForMarkedSourceNode(node: ts.Node): string {
    if (ts.isConstructorDeclaration(node)) {
      return 'constructor';
    }
    if ('name' in node) {
      return (node as ts.DeclarationStatement).name?.getText() ?? 'anonymous';
    }
    return 'anonymous';
  }

  collectContextPartsForMarkedSource(node: ts.Node): string[] {
    switch (node.kind) {
      case ts.SyntaxKind.PropertyDeclaration:
      case ts.SyntaxKind.PropertySignature:
      case ts.SyntaxKind.EnumMember:
      case ts.SyntaxKind.MethodDeclaration:
      case ts.SyntaxKind.MethodSignature:
      case ts.SyntaxKind.Constructor:
        // Parent is a class/interface/enum. Use their name as context.
        return [this.getIdentifierForMarkedSourceNode(node.parent)];
      case ts.SyntaxKind.Parameter:
        const method = node.parent;
        if (!ts.isMethodSignature(method)
            && !ts.isMethodDeclaration(method)
            && !ts.isConstructorDeclaration(method)
            && !ts.isFunctionDeclaration(method)) {
          // We expect that parent of parameter is always a method. If it is not
          // then return undefined so no context will be emitted.
          return [];
        }
        // If method has parent (class) then add it to context. So it looks like
        // 'ClassName.methodName'. Otherwise if it's a standalone function - just
        // return function name.
        const methodContext = this.collectContextPartsForMarkedSource(method);
        methodContext.push(this.getIdentifierForMarkedSourceNode(method));
        return methodContext;
    }
    // For all other nodes like variables, functions, classes don't use context
    // for now. Other languages use namespace or filename as context for those but
    // it doesn't provide much information.
    return [];
  }

  /**
   * This function builds CONTEXT node for marked source for a node. Context is what
   * the given node belongs to. For example for class method context is the name of the class.
   *
   * When the provided node doesn't have a context, e.g. variable, then this method returns null.
   *
   * Compared to other languages (Java, Go) context calculation here is simpler. We
   * don't produce fully qualified class name including package (as TS doesn't have a concept of
   * qualified name). Though we could add filename as namespace in future.
   *
   * We also don't handle nesting. In TS one can have class within a method within a class
   * within a method. It is possible to fully calculate such name (similar to what we do in
   * scopedSignature) but given that it's user-visible string - keep it simple.
   * Even though it's incomplete.
   */
  buildMarkedSourceContextNode(node: ts.Node): JSONMarkedSource|null {
    const parts = this.collectContextPartsForMarkedSource(node);
    if (parts.length === 0) {
      return null;
    } else {
      return {
        kind: MarkedSourceKind.CONTEXT,
        post_child_text: '.',
        add_final_list_token: true,
        child: parts.map(c => ({kind: MarkedSourceKind.IDENTIFIER, pre_text: c})),
      };
    }
  }

  /**
   * Emits a code fact for a variable or property declaration, specifying how
   * the declaration should be presented to users.
   *
   * The form of the code fact is
   *     ((property)|(local var)|const|let) <name>: <type>( = <initializer>)?
   * where `(local var)` is the declaration of a variable in a catch clause.
   */
  emitMarkedSourceForVariable(
      decl: ts.VariableDeclaration|ts.PropertyAssignment|
      ts.PropertyDeclaration|ts.BindingElement|ts.ShorthandPropertyAssignment|
      ts.PropertySignature|ts.JsxAttribute|ts.ParameterDeclaration|ts.EnumMember,
      declVName: VName) {
    const codeParts: JSONMarkedSource[] = [];
    let varDecl;
    const bindingPath: Array<string|number|undefined> = [];
    if (ts.isBindingElement(decl)) {
      // The node we want to emit code for is a BindingElement. This parent of
      // this is always a BindingPattern; the parent of the BindingPattern is
      // another BindingPattern, a ParameterDeclaration, or VariableDeclaration.
      // We handle ParameterDeclarations in `visitParameters`, so here we only
      // care about the declaration from a VariableDeclaration.
      bindingPath.push(this.bindingElemIndex(decl));
      varDecl = decl.parent.parent;
      while (!ts.isVariableDeclaration(varDecl) && varDecl !== undefined) {
        if (ts.isParameter(varDecl)) return;
        bindingPath.push(this.bindingElemIndex(varDecl));
        varDecl = varDecl.parent.parent;
      }
      if (varDecl === undefined) {
        todo(
            this.sourceRoot, decl,
            `Does not have variable and parameter declaration.`);
      }
    } else {
      varDecl = decl;
    }
    const context = this.buildMarkedSourceContextNode(decl);
    if (context != null) {
      codeParts.push(context);
    }
    codeParts.push({
      kind: MarkedSourceKind.IDENTIFIER,
      pre_text: fmtMarkedSource(this.getIdentifierForMarkedSourceNode(decl)),
    });
    if (!ts.isEnumMember(varDecl)) {
      const ty = this.typeChecker.getTypeAtLocation(decl);
      const tyStr = this.typeChecker.typeToString(ty, decl);
      codeParts.push(
        {kind: MarkedSourceKind.TYPE, pre_text: ': ', post_text: fmtMarkedSource(tyStr)});

    }
    if ('initializer' in varDecl && varDecl.initializer) {
      let init: ts.Node = varDecl.initializer;

      if (ts.isObjectLiteralExpression(init) ||
          ts.isArrayLiteralExpression(init)) {
        const narrowedInit = isNonNullableArray(bindingPath) &&
            this.walkObjectLikeLiteral(init, bindingPath.reverse());
        init = narrowedInit || init;
      }

      codeParts.push(
        {kind: MarkedSourceKind.INITIALIZER, pre_text: fmtMarkedSource(init.getText())});
    }

    const markedSource = {kind: MarkedSourceKind.BOX, child: codeParts};
    this.emitFact(declVName, FactName.CODE_JSON, JSON.stringify(markedSource));
  }

  /**
   * Emits a code fact for a class specifying how the declaration should be presented to users.
   */
  emitMarkedSourceForClasslikeDeclaration(
    decl: ts.ClassLikeDeclaration|ts.InterfaceDeclaration|ts.EnumDeclaration, declVName: VName) {
    const markedSource: JSONMarkedSource =
      {kind: MarkedSourceKind.IDENTIFIER, pre_text: this.getIdentifierForMarkedSourceNode(decl)};
    this.emitFact(declVName, FactName.CODE_JSON, JSON.stringify(markedSource));
  }

  /**
   * Emits a code fact for a function specifying how the declaration should be presented to users.
   */
  emitMarkedSourceForFunction(decl: ts.FunctionLikeDeclaration, declVName: VName) {
    const codeParts: JSONMarkedSource[] = [];
    const context = this.buildMarkedSourceContextNode(decl);
    if (context != null) {
      codeParts.push(context);
    }
    codeParts.push({
      kind: MarkedSourceKind.IDENTIFIER,
      pre_text: this.getIdentifierForMarkedSourceNode(decl),
    });
    codeParts.push({
      kind: MarkedSourceKind.PARAMETER_LOOKUP_BY_PARAM,
      pre_text: '(',
      post_child_text: ', ',
      post_text: ')'
    });
    const signature = this.typeChecker.getTypeAtLocation(decl).getCallSignatures()[0];
    if (signature) {
      const returnType = signature.getReturnType();
      const returnTypeStr = this.typeChecker.typeToString(returnType, decl);
      codeParts.push({
        kind: MarkedSourceKind.TYPE,
        pre_text: ': ',
        post_text: fmtMarkedSource(returnTypeStr),
      });
    }
    const markedSource = {kind: MarkedSourceKind.BOX, child: codeParts};
    this.emitFact(declVName, FactName.CODE_JSON, JSON.stringify(markedSource));
  }

  /**
   * Given a path of properties, walks the properties/elements of an
   * object/array literal, yielding the final node along the path.
   *
   * If the path or object literal is malformed (i.e. the property does not
   * exist or the object to walk is not a literal), nothing is returned.
   */
  walkObjectLikeLiteral(
      objLiteral: ts.ArrayLiteralExpression|ts.ObjectLiteralExpression,
      path: Array<string|number>,
      ): ts.Node|undefined {
    let node: ts.Node = objLiteral;
    for (const prop of path) {
      let next: ts.Node|undefined;
      if (ts.isObjectLiteralExpression(node)) {
        // The property name node text is the "index" of the property. See
        // `bindingElemIndex` for more details.
        next = node.properties.find(
            p => p.name && this.getPropertyNameStr(p.name) === prop);
        if (next && ts.isPropertyAssignment(next)) {
          next = next.initializer;
        }
      } else if (
          ts.isArrayLiteralExpression(node) && typeof prop === 'number') {
        next = (node as ts.ArrayLiteralExpression).elements[prop];
      }
      if (!next) {
        todo(
            this.sourceRoot, node,
            `expected to be an object-like literal with property '${prop}'`)
        return;
      }
      node = next;
    }
    return node;
  }

  /**
   * Given an object literal that is used in a place where given type is
   * expected - adds refs from the literals properties to the type's properties.
   */
  connectObjectLiteralToType(
      literal: ts.ObjectLiteralExpression, typeNode: ts.TypeNode|undefined) {
    if (typeNode == null) return;
    const type = this.typeChecker.getTypeFromTypeNode(typeNode);
    for (const prop of literal.properties) {
      if (ts.isPropertyAssignment(prop) ||
          ts.isShorthandPropertyAssignment(prop) ||
          ts.isMethodDeclaration(prop)) {
        this.emitPropertyRef(prop.name, type);
      }
    }
  }

  /**
   * Given object binding pattern like `const {name} = getPerson();` tries to
   * add refs from binding variables to the propeties of the type. Like `name`
   * is connected to `Person.name`.
   */
  connectObjectBindingPatternToType(binding: ts.ObjectBindingPattern) {
    const type = this.typeChecker.getTypeAtLocation(binding);
    for (const prop of binding.elements) {
      if (prop.propertyName) {
        this.emitPropertyRef(prop.propertyName, type);
      } else if (ts.isIdentifier(prop.name)) {
        this.emitPropertyRef(prop.name, type);
      }
    }
  }

  /**
   * Helper function to emit `ref` node from a given property (usually of object
   * literal or destructuring pattern) to the type. Checks whether provided type
   * contains a property with the same name and will use it for `ref` node.
   */
  emitPropertyRef(prop: ts.PropertyName, type: ts.Type) {
    const propName = this.getPropertyNameStr(prop);
    if (propName == null) return;
    const propertyOnType = type.getProperty(propName);
    if (propertyOnType == null) return;
    const propFlags = propertyOnType.flags;
    const isType = (propFlags &
                    (ts.SymbolFlags.Class | ts.SymbolFlags.Interface |
                     ts.SymbolFlags.RegularEnum | ts.SymbolFlags.TypeAlias)) > 0;
    const vname = this.host.getSymbolName(
        propertyOnType, isType ? TSNamespace.TYPE : TSNamespace.VALUE);
    if (vname == null) return;
    const anchor = this.newAnchor(prop);
    this.emitEdge(anchor, EdgeKind.REF_ID, vname);
  }

  /**
   * Returns the property "index" of a bound element in a binding pattern, if
   * known. For example,
   * - `1` has index `2` in the binding pattern `[3, 2, 1]`
   * - `c` has index `c` in the binding pattern `{a, b, c}`
   * - `calias` has index `c` in the binding pattern `{a, b, c: calias}`
   */
  bindingElemIndex(elem: ts.BindingElement): string|number|undefined {
    const bindingPat = elem.parent;
    if (ts.isObjectBindingPattern(bindingPat)) {
      if (elem.propertyName) {
        return this.getPropertyNameStr(elem.propertyName);
      }
      switch (elem.name.kind) {
        case ts.SyntaxKind.Identifier:
          return elem.name.text;
        case ts.SyntaxKind.ArrayBindingPattern:
        case ts.SyntaxKind.ObjectBindingPattern:
          return undefined;
      }
    } else {
      return bindingPat.elements.indexOf(elem);
    }
  }

  /**
   * Returns the string content of a property name, if known.
   * The name of complex computed properties is often not known.
   */
  getPropertyNameStr(elem: ts.PropertyName): string|undefined {
    switch (elem.kind) {
      case ts.SyntaxKind.Identifier:
      case ts.SyntaxKind.StringLiteral:
      case ts.SyntaxKind.NumericLiteral:
      case ts.SyntaxKind.PrivateIdentifier:
        return elem.text;
      case ts.SyntaxKind.ComputedPropertyName:
        const name = this.host.getSymbolAtLocation(elem)?.name;
        // If the computed property expression is more complicated than an
        // identifier (e.g. `['red' + 'cat']` or `[fn()]`), the name isn't
        // resolved and the symbol name is marked as "__computed". This doesn't
        // help us for indexing, so return "undefined" in such cases.
        // Constant evaluation of the computed property name is not always
        // possible (e.g. the expression may be a function call), and usage of
        // computed properties is probably rare enough that handling the "simple
        // case" is good enough for now.
        return name !== '__computed' ? name : undefined;
    }
  }

  /**
   * Emit "overrides" edges if this method overrides extended classes or
   * implemented interfaces, which are listed in the Heritage Clauses of
   * a class or interface.
   *     class X extends A implements B, C {}
   *             ^^^^^^^^^-^^^^^^^^^^^^^^^----- `HeritageClause`s
   * Look at each type listed in the heritage clauses and traverse its
   * members. If the type has a member that matches the method visited in
   * this function (`kFunc`), emit an "overrides" edge to that member.
   */
  emitOverridesEdgeForFunction(
      funcSym: ts.Symbol, funcVName: VName,
      parent: ts.ClassLikeDeclaration|ts.InterfaceDeclaration) {
    if (parent.heritageClauses == null) {
      return;
    }
    for (const heritage of parent.heritageClauses) {
      for (const baseType of heritage.types) {
        const type = this.typeChecker.getTypeAtLocation(baseType.expression);
        if (!type || !type.symbol || !type.symbol.members) {
          continue;
        }

        const funcName = funcSym.name;
        const funcFlags = funcSym.flags;

        // Find a member of with the same type (same flags) and same name
        // as the overriding method.
        const overriddenCondition = (sym: ts.Symbol) =>
            Boolean(sym.flags & funcFlags) && sym.name === funcName;

        const overridden = toArray<ts.Symbol>(type.symbol.members.values())
            .find(overriddenCondition);
        if (overridden) {
          const base = this.host.getSymbolName(overridden, TSNamespace.VALUE);
          if (base) {
            this.emitEdge(funcVName, EdgeKind.OVERRIDES, base);
          }
        } else {
          // If parent class or interface doesn't have this method - it's possible
          // that parent's parent might. To check for that recurse to the parent's parent
          // classes/interfaces.
          const decl = type.symbol.declarations?.[0];
          if (decl && (ts.isClassLike(decl) || ts.isInterfaceDeclaration(decl))) {
            this.emitOverridesEdgeForFunction(funcSym, funcVName, decl);
          }
        }
      }
    }
  }

  visitFunctionLikeDeclaration(decl: ts.FunctionLikeDeclaration) {
    this.visitDecorators(ts.canHaveDecorators(decl) ? ts.getDecorators(decl) : []);
    const {sym, vname} = this.getSymbolAndVNameForFunctionDeclaration(decl);
    if (!vname) {
      todo(
          this.sourceRoot, decl,
          `function declaration ${decl.getText()} has no symbol`);
      return;
    }
    const isNameComputedProperty =
        decl.name && decl.name.kind === ts.SyntaxKind.ComputedPropertyName;
    if (isNameComputedProperty) {
      this.visit((decl.name as ts.ComputedPropertyName).expression);
    }

    // Treat functions with computed names as anonymous. From developer point of
    // view in the following expression: `const k = {[foo]() {}};` the part
    // `[foo]` isn't a separate symbol that you can click. Only `foo` should be
    // xref'ed and lead to the `foo` definition.
    if (decl.name && !isNameComputedProperty) {
      if (!sym) {
        todo(
            this.sourceRoot, decl.name,
            `function declaration ${decl.name.getText()} has no symbol`);
        return;
      }

      const declAnchor = this.newAnchor(decl.name);
      this.emitNode(vname, NodeKind.FUNCTION);
      this.emitEdge(declAnchor, EdgeKind.DEFINES_BINDING, vname);

      // Getters/setters also emit an implicit class property entry. If a
      // getter is present, it will bind this entry; otherwise a setter will.
      if (ts.isGetAccessor(decl) ||
          (ts.isSetAccessor(decl) &&
           (!sym.declarations || !sym.declarations.find(ts.isGetAccessor)))) {
        this.emitImplicitProperty(decl, declAnchor, vname);
      }

      this.visitJSDoc(decl, vname, false);
    }
    this.emitEdge(this.newAnchor(decl), EdgeKind.DEFINES, vname);

    if (decl.parent) {
      this.emitChildofEdge(vname, decl.parent);
      if ((ts.isClassLike(decl.parent) ||
           ts.isInterfaceDeclaration(decl.parent)) &&
          sym) {
        this.emitOverridesEdgeForFunction(sym, vname, decl.parent);
      }
    }

    this.visitParameters(decl.parameters, vname);

    if (decl.type) {
      // "type" here is the return type of the function.
      this.visitType(decl.type);
    }

    if (decl.typeParameters)
      this.visitTypeParameters(vname, decl.typeParameters);
    if (decl.body) {
      this.visit(decl.body);
    } else {
      this.emitFact(vname, FactName.COMPLETE, 'incomplete');
    }
    this.emitMarkedSourceForFunction(decl, vname);
  }

  /**
   * Emits childof edge from member to their parents. Parent can be class/interface/enum.
   * See https://kythe.io/docs/schema/#childof
   */
  emitChildofEdge(vname: VName, parent: ts.Node) {
    if (!ts.isClassLike(parent) && !ts.isInterfaceDeclaration(parent) &&
        !ts.isEnumDeclaration(parent)) {
      return;
    }
    const parentName = parent.name;
    if (parentName == null) {
      return;
    }
    const parentSym = this.host.getSymbolAtLocationFollowingAliases(parentName);
    if (!parentSym) {
      todo(this.sourceRoot, parentName, `parent ${parentName} has no symbol`);
      return;
    }
    const kParent = this.host.getSymbolName(parentSym, TSNamespace.TYPE);
    if (kParent) {
      this.emitEdge(vname, EdgeKind.CHILD_OF, kParent);
    }
  }

  /**
   * Visits a function parameters, which can be recursive in the case of
   * parameters created via bound elements:
   *    function foo({a, b: {c, d}}, e, f) {}
   * In this code, a, c, d, e, f are all parameters with increasing parameter
   * numbers [0, 4].
   */
  visitParameters(
      parameters: ReadonlyArray<ts.ParameterDeclaration>, kFunc: VName) {
    let paramNum = 0;
    const recurseVisit =
        (param: ts.ParameterDeclaration|ts.BindingElement) => {
          this.visitDecorators(ts.canHaveDecorators(param) ? ts.getDecorators(param) : []);

          switch (param.name.kind) {
            case ts.SyntaxKind.Identifier:
              const sym = this.host.getSymbolAtLocationFollowingAliases(param.name);
              if (!sym) {
                todo(
                    this.sourceRoot, param.name,
                    `param ${param.name.getText()} has no symbol`);
                return;
              }
              const kParam = this.host.getSymbolName(sym, TSNamespace.VALUE);
              if (!kParam) return;
              this.emitNode(kParam, NodeKind.VARIABLE);

              this.emitEdge(
                  kFunc, makeOrdinalEdge(EdgeKind.PARAM, paramNum), kParam);
              ++paramNum;

              if (ts.isParameterPropertyDeclaration(param, param.parent)) {
                // Class members defined in the parameters of a constructor are
                // children of the class type.
                const parentName = param.parent.parent.name;
                if (parentName !== undefined) {
                  const parentSym = this.host.getSymbolAtLocationFollowingAliases(parentName);
                  if (parentSym !== undefined) {
                    const kClass =
                        this.host.getSymbolName(parentSym, TSNamespace.TYPE);
                    if (!kClass) return;
                    this.emitEdge(kParam, EdgeKind.CHILD_OF, kClass);
                  }
                }
              } else {
                this.emitEdge(kParam, EdgeKind.CHILD_OF, kFunc);
              }

              this.emitEdge(
                  this.newAnchor(param.name), EdgeKind.DEFINES_BINDING, kParam);
              this.emitMarkedSourceForVariable(param, kParam);
              break;
            case ts.SyntaxKind.ObjectBindingPattern:
            case ts.SyntaxKind.ArrayBindingPattern:
              const elements = toArray(param.name.elements.entries());
              for (const [index, element] of elements) {
                if (ts.isBindingElement(element)) {
                  recurseVisit(element);
                }
              }
              // Handles case like:
              // funtion doSomething({name}: Person) { ... }
              // and adds refs from `name` variable to the Person.name field.
              if (ts.isObjectBindingPattern(param.name)) {
                this.connectObjectBindingPatternToType(param.name);
              }
              break;
            default:
              break;
          }

          if (ts.isParameter(param) && param.type) this.visitType(param.type);
          if (param.initializer) this.visit(param.initializer);
        }

    for (const element of parameters) {
      recurseVisit(element);
    }
  }

  visitDecorators(decors: ReadonlyArray<ts.Decorator> | undefined) {
    if (decors) {
      for (const decor of decors) {
        this.visit(decor);
      }
    }
  }

  /**
   * Visits a module declaration, which can look like any of the following:
   *     declare module 'foo';
   *     declare module 'foo' {}
   *     declare module foo {}
   *     namespace Foo {}
   */
  visitModuleDeclaration(decl: ts.ModuleDeclaration) {
    let sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) {
      todo(
          this.sourceRoot, decl.name,
          `module declaration ${decl.name.getText()} has no symbol`);
      return;
    }
    // A TypeScript module declaration declares both a namespace (a Kythe
    // "record") and a value (most similar to a "package", which defines a
    // module with declarations).
    const kNamespace = this.host.getSymbolName(sym, TSNamespace.NAMESPACE);
    const kValue = this.host.getSymbolName(sym, TSNamespace.VALUE);
    if (!kNamespace || !kValue) return;
    // It's possible that same namespace appears multiple time. We need to
    // emit only single node for that namespace and single defines/binding
    // edge.
    if (sym.valueDeclaration === decl) {
      this.emitNode(kNamespace, NodeKind.RECORD);
      this.emitSubkind(kNamespace, Subkind.NAMESPACE);
      this.emitNode(kValue, NodeKind.PACKAGE);

      const nameAnchor = this.newAnchor(decl.name);
      this.emitEdge(nameAnchor, EdgeKind.DEFINES_BINDING, kNamespace);
      this.emitEdge(nameAnchor, EdgeKind.DEFINES_BINDING, kValue);
      // If no body then it is incomplete module definition, like declare module
      // 'foo';
      this.emitFact(
          kNamespace, FactName.COMPLETE,
          decl.body ? 'definition' : 'incomplete');
    }

    // The entire module declaration defines the created namespace.
    this.emitEdge(this.newAnchor(decl), EdgeKind.DEFINES, kValue);

    this.visitDecorators(ts.canHaveDecorators(decl) ? ts.getDecorators(decl) : []);
    if (decl.body) this.visit(decl.body);
  }

  visitClassDeclaration(decl: ts.ClassDeclaration) {
    this.visitDecorators(ts.canHaveDecorators(decl) ? ts.getDecorators(decl) : []);
    let kClass: VName|undefined;
    if (decl.name) {
      const sym = this.host.getSymbolAtLocation(decl.name);
      if (!sym) {
        todo(
            this.sourceRoot, decl.name,
            `class ${decl.name.getText()} has no symbol`);
        return;
      }
      // A 'class' declaration declares both a type (a 'record', representing
      // instances of the class) and a value (least ambigiously, also the
      // class declaration).
      kClass = this.host.getSymbolName(sym, TSNamespace.TYPE);
      if (!kClass) return;
      this.emitNode(kClass, NodeKind.RECORD);
      const anchor = this.newAnchor(decl.name);
      this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kClass);

      // Emit constructor.
      const kCtor = this.host.getSymbolName(sym, TSNamespace.VALUE);
      if (!kCtor) return;
      let ctorAnchor = anchor;
      // If the class has an explicit constructor method - use it as an anchor.
      const ctorSymbol = this.getCtorSymbol(decl);
      if (ctorSymbol && ctorSymbol.declarations) {
        const ctorDecl = ctorSymbol.declarations[0];
        const span = this.getTextSpan(ctorDecl, 'constructor');
        ctorAnchor = this.newAnchor(ctorDecl, span.start, span.end);
      }
      this.emitNode(kCtor, NodeKind.FUNCTION);
      this.emitSubkind(kCtor, Subkind.CONSTRUCTOR);
      this.emitEdge(ctorAnchor, EdgeKind.DEFINES_BINDING, kCtor);

      this.visitJSDoc(decl, kClass);
      this.emitMarkedSourceForClasslikeDeclaration(decl, kClass);
    }
    if (decl.typeParameters)
      this.visitTypeParameters(kClass, decl.typeParameters);
    if (decl.heritageClauses) this.visitHeritage(kClass, decl.heritageClauses);
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  visitEnumDeclaration(decl: ts.EnumDeclaration) {
    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) return;
    const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
    if (!kType) return;
    this.emitNode(kType, NodeKind.RECORD);
    const kValue = this.host.getSymbolName(sym, TSNamespace.VALUE);
    if (!kValue) return;
    this.emitNode(kValue, NodeKind.CONSTANT);

    const anchor = this.newAnchor(decl.name);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kType);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kValue);
    for (const member of decl.members) {
      this.visit(member);
    }
    this.emitMarkedSourceForClasslikeDeclaration(decl, kValue);
  }

  visitEnumMember(decl: ts.EnumMember) {
    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) return;
    const kMember = this.host.getSymbolName(sym, TSNamespace.VALUE);
    if (!kMember) return;
    this.emitNode(kMember, NodeKind.CONSTANT);
    this.emitEdge(this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, kMember);
    this.emitMarkedSourceForVariable(decl, kMember);
    this.emitChildofEdge(kMember, decl.parent);
  }

  visitExpressionMember(node: ts.Node) {
    const sym = this.host.getSymbolAtLocationFollowingAliases(node);
    if (!sym) {
      // E.g. a field of an "any".
      return;
    }
    if (!sym.declarations || sym.declarations.length === 0) {
      // An undeclared symbol, e.g. "undefined".
      return;
    }
    const isClass = (sym.flags & ts.SymbolFlags.Class) > 0;
    const isConstructorCall = ts.isNewExpression(node.parent);
    const ns = isClass && !isConstructorCall ? TSNamespace.TYPE : TSNamespace.VALUE;
    const name = this.host.getSymbolName(sym, ns);
    if (!name) return;
    const anchor = this.newAnchor(node);

    const refType = this.getRefType(node, sym);
    if (refType == RefType.READ || refType == RefType.READ_WRITE) {
      this.emitEdge(anchor, EdgeKind.REF, name);
    }
    if (refType == RefType.WRITE || refType == RefType.READ_WRITE) {
      this.emitEdge(anchor, EdgeKind.REF_WRITES, name);
    }
    // For classes emit ref/id to the type node in addition to regular
    // ref. When user check refs for a class - they usually check get
    // refs of the class node, not the constructor node. That's why
    // we need ref/id from all usages to the class node.
    if (isConstructorCall) {
      const className = this.host.getSymbolName(sym, TSNamespace.TYPE);
      if (className != null) {
        this.emitEdge(anchor, EdgeKind.REF_ID, className);
      }
    }
    this.addInfluencer(name);
  }

  /**
   * Determines the type of reference type of a {@link ts.Node} in its parent
   * expression.
   * @param node The {@link ts.Node} being referenced
   * @param sym The {@link ts.Symbol} associated with the node
   * @returns The {@link RefType} indicating whether the reference is READ,
   * WRITE, or READ_WRITE
   */
  getRefType(node: ts.Node, sym: ts.Symbol): RefType {
    // If the identifier being accessed is a property of a class, we need to
    // recurse through the parents nodes until we get the true parent
    // expression.
    let parent = node.parent;
    while (ts.isPropertyAccessExpression(parent)) {
      parent = parent.parent;
    }

    if (ts.isPrefixUnaryExpression(parent) ||
        ts.isPostfixUnaryExpression(parent)) {
      let operator: ts.SyntaxKind = parent.operator;
      let operand: ts.Expression = parent.operand;

      const operandNode = ts.isPropertyAccessExpression(operand) ?
          this.propertyAccessIdentifier(operand) as ts.Node :
          operand as ts.Node;

      // Check if the operand is the same as referenced symbol and if this was
      // an increment or decrement operation. If both are true, this is a
      // READ_WRITE reference.
      if (this.expressionMatchesSymbol(operand, sym) &&
          operandNode.pos === node.pos) {
        if (operator === ts.SyntaxKind.PlusPlusToken ||
            operator === ts.SyntaxKind.MinusMinusToken) {
          return RefType.READ_WRITE;
        }
      }
    } else if (ts.isBinaryExpression(parent)) {
      const lhsNode = ts.isPropertyAccessExpression(parent.left) ?
          this.propertyAccessIdentifier(parent.left) as ts.Node :
          parent.left as ts.Node;
      const opString = ts.tokenToString(parent.operatorToken.kind) || '';

      if (this.expressionMatchesSymbol(parent.left, sym) &&
          lhsNode.pos === node.pos) {
        const compoundAssignmentOperators = [
          '+=', '-=', '*=', '**=', '/=', '%=', '<<=', '>>=', '>>>=', '&=', '|=',
          '^='
        ];
        if (compoundAssignmentOperators.includes(opString)) {
          // Compound assignment operations are always a READ_WRITE reference
          return RefType.READ_WRITE;
        } else if (opString === '=') {
          // If the symbol is on the left side of a assignment operation, it
          // is always at least a WRITE reference. However, we then need to
          // check if the parent expression is also a binary expression. If so,
          // we check whether the current binary expression is on the right
          // side of that expression, indicating a chained assignment such as
          // x = y = z. If this is the case, the reference becomes READ_WRITE
          // instead.
          if (parent.parent !== null && ts.isBinaryExpression(parent.parent)) {
            if (ts.tokenToString(parent.parent.operatorToken.kind) === '=' &&
                parent.parent.right === parent) {
              return RefType.READ_WRITE;
            }
          }
          return RefType.WRITE;
        }
      }
    }
    return RefType.READ;
  }

  /**
   * Check whether a {@link ts.Expression} matches a symbol. If the expression
   * is a {@link ts.PropertyAccessExpression}, the check is performed against
   * the base property being accessed.
   */
  expressionMatchesSymbol(expression: ts.Expression, symbol: ts.Symbol):
      boolean {
    if (ts.isIdentifier(expression)) {
      return this.host.getSymbolAtLocation(expression) === symbol;
    } else if (ts.isPropertyAccessExpression(expression)) {
      return this.host.getSymbolAtLocation(
                 this.propertyAccessIdentifier(expression)) === symbol;
    }
    return false;
  }


  /**
   * Recurses through a {@link ts.PropertyAccessExpression} to find the
   * identifier of the base property being accessed. Recursion is necessary
   * because a chained property accesses such as `obj.member.member` will
   * have two layers of {@link ts.PropertyAccessExpression}s.
   *
   * @param expression The {@link ts.PropertyAccessExpression} to process
   * @returns The identifier of the base property being accessed
   */
  propertyAccessIdentifier(expression: ts.PropertyAccessExpression):
      ts.Identifier|ts.PrivateIdentifier {
    while (ts.isPropertyAccessExpression(expression.parent)) {
      expression = expression.parent;
    }
    return expression.name;
  }

  /**
   * Emits a reference from a "this" keyword to the type of the "this" object.
   */
  visitThisKeyword(keyword: ts.ThisExpression) {
    const sym = this.host.getSymbolAtLocation(keyword);
    if (!sym) {
      // "this" refers to an object with no particular type, e.g.
      //   let obj = {
      //     foo() { this.foo(); }
      //   };
      return;
    }
    if (!sym.declarations || sym.declarations.length === 0) {
      // "this" keyword is `globalThis`, which has no declarations.
      return;
    }

    const type = this.host.getSymbolName(sym, TSNamespace.TYPE);
    if (!type) return;
    const thisAnchor = this.newAnchor(keyword);
    this.emitEdge(thisAnchor, EdgeKind.REF, type);
  }

  /**
   * visitJSDoc attempts to attach a 'doc' node to a given target, by looking
   * for JSDoc comments.
   */
  visitJSDoc(node: ts.Node, target: VName, emitDeprecation: boolean = true) {
    if (emitDeprecation) {
      this.maybeTagDeprecated(node, target);
    }

    const text = node.getFullText();
    const comments = ts.getLeadingCommentRanges(text, 0);
    if (!comments) return;

    let jsdoc: string|undefined;
    for (const commentRange of comments) {
      if (commentRange.kind !== ts.SyntaxKind.MultiLineCommentTrivia) continue;
      const comment =
          text.substring(commentRange.pos + 2, commentRange.end - 2);
      if (!comment.startsWith('*')) {
        // Not a JSDoc comment.
        continue;
      }
      // Strip the ' * ' bits that start lines within the comment.
      jsdoc = comment.replace(/^[ \t]*\* ?/mg, '');
      break;
    }
    if (jsdoc === undefined) return;

    // Strip leading and trailing whitespace.
    jsdoc = jsdoc.replace(/^\s+/, '').replace(/\s+$/, '');
    const doc = this.newVName(target.signature + '#doc', target.path);
    this.emitNode(doc, NodeKind.DOC);
    this.emitEdge(doc, EdgeKind.DOCUMENTS, target);
    this.emitFact(doc, FactName.TEXT, jsdoc);
  }

  visitAsExpressions(node: ts.AsExpression) {
    ts.forEachChild(node, (child) => {
      this.visit(child);
    });
    // Handle case like `{name: 'Alice'} as Person` and connect `name` property
    // to Person.name.
    if (ts.isObjectLiteralExpression(node.expression)) {
      this.connectObjectLiteralToType(node.expression, node.type);
    }
  }

  /**
   * Tags a node as deprecated if its JSDoc marks it as so.
   * TODO(TS 4.0): TS 4.0 exposes a JSDocDeprecatedTag.
   */
  maybeTagDeprecated(node: ts.Node, nodeVName: VName) {
    const deprecatedTag =
        ts.getJSDocTags(node).find(tag => tag.tagName.text === 'deprecated');
    if (deprecatedTag) {
      this.emitFact(
          nodeVName, FactName.TAG_DEPRECATED,
          (deprecatedTag.comment || '') as any);
    }
  }

  /** visit is the main dispatch for visiting AST nodes. */
  visit(node: ts.Node): void {
    switch (node.kind) {
      case ts.SyntaxKind.ImportDeclaration:
      case ts.SyntaxKind.ImportEqualsDeclaration:
        return this.visitImportDeclaration(
            node as ts.ImportDeclaration | ts.ImportEqualsDeclaration);
      case ts.SyntaxKind.ExportAssignment:
        return this.visitExportAssignment(node as ts.ExportAssignment);
      case ts.SyntaxKind.ExportDeclaration:
        return this.visitExportDeclaration(node as ts.ExportDeclaration);
      case ts.SyntaxKind.VariableStatement:
        return this.visitVariableStatement(node as ts.VariableStatement);
      case ts.SyntaxKind.VariableDeclaration:
        this.visitVariableDeclaration(node as ts.VariableDeclaration);
        return;
      case ts.SyntaxKind.PropertyAssignment:  // property in object literal
      case ts.SyntaxKind.PropertyDeclaration:
      case ts.SyntaxKind.PropertySignature:
      case ts.SyntaxKind.ShorthandPropertyAssignment:
        const vname =
            this.visitVariableDeclaration(node as ts.PropertyDeclaration);
        if (vname) this.visitJSDoc(node, vname);
        return;
      case ts.SyntaxKind.ArrowFunction:
      case ts.SyntaxKind.Constructor:
      case ts.SyntaxKind.FunctionDeclaration:
      case ts.SyntaxKind.FunctionExpression:
      case ts.SyntaxKind.MethodDeclaration:
      case ts.SyntaxKind.MethodSignature:
      case ts.SyntaxKind.GetAccessor:
      case ts.SyntaxKind.SetAccessor:
        return this.visitFunctionLikeDeclaration(
            node as ts.FunctionLikeDeclaration);
      case ts.SyntaxKind.ClassDeclaration:
        return this.visitClassDeclaration(node as ts.ClassDeclaration);
      case ts.SyntaxKind.InterfaceDeclaration:
        return this.visitInterfaceDeclaration(node as ts.InterfaceDeclaration);
      case ts.SyntaxKind.TypeAliasDeclaration:
        return this.visitTypeAliasDeclaration(node as ts.TypeAliasDeclaration);
      case ts.SyntaxKind.EnumDeclaration:
        return this.visitEnumDeclaration(node as ts.EnumDeclaration);
      case ts.SyntaxKind.EnumMember:
        return this.visitEnumMember(node as ts.EnumMember);
      case ts.SyntaxKind.TypeReference:
        this.visitType(node as ts.TypeNode);
        return;
      case ts.SyntaxKind.BindingElement:
        this.visitVariableDeclaration(node as ts.BindingElement);
        return;
      case ts.SyntaxKind.JsxAttribute:
        // TODO: go/ts51upgrade - Auto-added to unblock TS5.1 migration.
        //   TS2345: Argument of type 'JsxAttribute' is not assignable to parameter of type '{ name: ObjectBindingPattern | ArrayBindingPattern | Identifier | StringLiteral | NumericLiteral | ComputedPropertyName | PrivateIdentifier; type?: TypeNode | undefined; initializer?: Expression | undefined; kind: SyntaxKind; } & Node'.
        // @ts-ignore
        this.visitVariableDeclaration(node as ts.JsxAttribute);
        return;
      case ts.SyntaxKind.Identifier:
      case ts.SyntaxKind.StringLiteral:
      case ts.SyntaxKind.NumericLiteral:
        // Assume that this identifer is occurring as part of an
        // expression; we handle identifiers that occur in other
        // circumstances (e.g. in a type) separately in visitType.
        this.visitExpressionMember(node);
        return;
      case ts.SyntaxKind.ThisKeyword:
        return this.visitThisKeyword(node as ts.ThisExpression);
      case ts.SyntaxKind.ModuleDeclaration:
        return this.visitModuleDeclaration(node as ts.ModuleDeclaration);
      case ts.SyntaxKind.CallExpression:
      case ts.SyntaxKind.NewExpression:
        this.visitCallOrNewExpression(
            node as ts.CallExpression | ts.NewExpression);
        return;
      case ts.SyntaxKind.ReturnStatement:
        this.visitReturnStatement(node as ts.ReturnStatement)
        return;
      case ts.SyntaxKind.AsExpression:
        this.visitAsExpressions(node as ts.AsExpression);
        return;
      default:
        // Use default recursive processing.
        return ts.forEachChild(node, n => this.visit(n));
    }
  }

  /** index is the main entry point, starting the recursive visit. */
  index() {
    this.emitFact(this.kFile, FactName.NODE_KIND, NodeKind.FILE);
    this.emitFact(this.kFile, FactName.TEXT, this.file.text);

    this.emitModuleAnchor(this.file);

    // Emit file-level init function to contain all call anchors that
    // don't have parent functions.
    const fileInitFunc = this.getSyntheticFileInitVName();
    this.emitFact(fileInitFunc, FactName.NODE_KIND, NodeKind.FUNCTION);
    this.emitEdge(
        this.newAnchor(this.file, 0, 0), EdgeKind.DEFINES, fileInitFunc);

    ts.forEachChild(this.file, n => this.visit(n));
  }
}

/**
 * Main plugin that runs over all srcs files in a compilation unit and emits
 * Kythe data for all symbols in those files.
 */
class TypescriptIndexer implements Plugin {
  name = 'TypescriptIndexerPlugin';

  index(context: IndexerHost) {
    for (const path of context.compilationUnit.srcs) {
      const sourceFile = context.program.getSourceFile(path);
      if (!sourceFile) {
        throw new Error(`requested indexing ${path} not found in program`);
      }
      const visitor = new Visitor(context, sourceFile);
      visitor.index();
    }
  }
}

/**
 * index indexes a TypeScript program, producing Kythe JSON objects for the
 * source files in the specified paths.
 *
 * (A ts.Program is a configured collection of parsed source files, but
 * the caller must specify the source files within the program that they want
 * Kythe output for, because e.g. the standard library is contained within
 * the Program and we only want to process it once.)
 *
 * @param paths Files to index
 */
export function index(compilationUnit: CompilationUnit, options: IndexingOptions): ts.Diagnostic[] {
  const program = ts.createProgram({
    rootNames: compilationUnit.rootFiles,
    options: options.compilerOptions,
    host: options.compilerHost,
  });
  // Note: we only call getPreEmitDiagnostics (which causes type checking to
  // happen) on the input paths as provided in paths.  This means we don't
  // e.g. type-check the standard library unless we were explicitly told to.
  const diags: ts.Diagnostic[] = [];
  for (const path of compilationUnit.srcs) {
    for (const diag of ts.getPreEmitDiagnostics(program, program.getSourceFile(path))) {
      diags.push(diag);
    }
  }
  // Note: don't abort if there are diagnostics.  This allows us to
  // index programs with errors.  We return these diagnostics at the end
  // so the caller can act on them if it wants.

  const indexingContext =  new StandardIndexerContext(program, compilationUnit, options);
  const plugins = [new TypescriptIndexer(), ...(options.plugins ?? [])];
  for (const plugin of plugins) {
    try {
      plugin.index(indexingContext);
    } catch (err) {
      console.error(`Plugin ${plugin.name} errored: ${err}`);
    }
  }

  return diags;
}

/**
 * loadTsConfig loads a tsconfig.json from a path, throwing on any errors
 * like "file not found" or parse errors.
 */
export function loadTsConfig(
    tsconfigPath: string, projectPath: string,
    host: ts.ParseConfigHost = ts.sys): ts.ParsedCommandLine {
  projectPath = path.resolve(projectPath);
  const {config: json, error} = ts.readConfigFile(tsconfigPath, host.readFile);
  if (error) {
    throw new Error(ts.formatDiagnostics([error], ts.createCompilerHost({})));
  }
  const config = ts.parseJsonConfigFileContent(json, host, projectPath);
  if (config.errors.length > 0) {
    throw new Error(
        ts.formatDiagnostics(config.errors, ts.createCompilerHost({})));
  }
  return config;
}

function main(argv: string[]) {
  if (argv.length < 1) {
    console.error('usage: indexer path/to/tsconfig.json [PATH...]');
    return 1;
  }

  const config = loadTsConfig(argv[0], path.dirname(argv[0]));
  let inPaths = argv.slice(1);
  if (inPaths.length === 0) {
    inPaths = config.fileNames;
  }

  // This program merely demonstrates the API, so use a fake corpus/root/etc.
  const rootVName: VName = {
    corpus: 'corpus',
    root: '',
    path: '',
    signature: '',
    language: '',
  };
  const compilationUnit: CompilationUnit = {
    srcs: inPaths,
    rootVName,
    rootFiles: inPaths,
    fileVNames: new Map(),
  };
  index(compilationUnit, {
    compilerOptions: config.options,
    compilerHost: ts.createCompilerHost(config.options),
    emit(obj: JSONFact | JSONEdge) {
      console.log(JSON.stringify(obj));
    }
  });
  return 0;
}

if (require.main === module) {
  // Note: do not use process.exit(), because that does not ensure that
  // process.stdout has been flushed(!).
  process.exitCode = main(process.argv.slice(2));
}
