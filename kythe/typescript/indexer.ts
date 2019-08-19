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

import {EdgeKind, FactName, JSONEdge, JSONFact, makeOrdinalEdge, OrdinalEdge, Subkind, VName} from './kythe';
import * as utf8 from './utf8';

const LANGUAGE = 'typescript';

/**
 * An indexer host holds information about the program indexing and methods
 * used by the TypeScript indexer that may also be useful to plugins, reducing
 * code duplication.
 */
export interface IndexerHost {
  /**
   * Gets the offset table for a file path.
   * These are used to lookup UTF-8 offsets (used by Kythe) from UTF-16 offsets
   * (used by TypeScript), and vice versa.
   */
  getOffsetTable(path: string): Readonly<utf8.OffsetTable>;
  /**
   * getSymbolAtLocation is the same as ts.TypeChecker.getSymbolAtLocation,
   * except that it has a return type that properly captures that
   * getSymbolAtLocation can return undefined.  (The TypeScript API itself is
   * not yet null-safe, so it hasn't been annotated with full types.)
   */
  getSymbolAtLocation(node: ts.Node): ts.Symbol|undefined;
  /**
   * Computes the VName (and signature) of a ts.Symbol. A Context can be
   * optionally specified to help disambiguate nodes with multiple declarations.
   * See the documentation of Context for more information.
   */
  getSymbolName(sym: ts.Symbol, ns: TSNamespace, context?: Context): VName;
  /**
   * Aliases a local symbol to a remote symbol. Returns the aliased name.
   * This is used to return a remote symbol's VNames for a local symbol when a
   * symbol is imported into a file; @see Visitor#visitImport.
   */
  aliasSymbol(localSym: ts.Symbol, remoteSym: ts.Symbol): VName;
  /**
   * scopedSignature computes a scoped name for a ts.Node.
   * E.g. if you have a function `foo` containing a block containing a variable
   * `bar`, it might return a VName like
   *   signature: "foo.block0.bar""
   *   path: <appropriate path to module>
   */
  scopedSignature(startNode: ts.Node): VName;
  /**
   * Converts a file path into a file VName.
   */
  pathToVName(path: string): VName;
  /**
   * Returns the module name of a TypeScript source file.
   * See moduleName() for more details.
   */
  moduleName(path: string): string;
  /**
   * Paths to index.
   */
  paths: string[];
  /**
   * TypeScript program.
   */
  program: ts.Program;
  /**
   * Strategy to emit Kythe entries by.
   *
   * TODO(ayazhafiz): change type to `JSONFact|JSONEdge` after downstream
   * clients are updated to use Kythe types.
   */
  emit(obj: {}): void;
}

/**
 * A indexer plugin adds extra functionality with the same inputs as the base
 * indexer.
 */
export interface Plugin {
  name: string;
  /**
   * Indexes a TypeScript program with extra functionality.
   * Takes a indexer host, which provides useful properties and methods that
   * the plugin can defer to rather than reimplementing.
   */
  index(context: IndexerHost): void;
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
 * stripExtension strips the .d.ts or .ts extension from a path.
 * It's used to map a file path to the module name.
 */
function stripExtension(path: string): string {
  return path.replace(/\.(d\.)?ts$/, '');
}

/**
 * TSNamespace represents the three declaration namespaces of TypeScript: types,
 * values, and (confusingly) namespaces. A given symbol may be a type, and/or a
 * value, and/or a namespace.
 *
 * See the table at
 *   https://www.typescriptlang.org/docs/handbook/declaration-merging.html
 * for a listing of namespace groups for various declaration types and further
 * discussion.
 */
export enum TSNamespace {
  TYPE,
  VALUE,
  NAMESPACE,
}

/**
 * Context represents the environment a node is declared in, and may be used for
 * disambiguating a node's declarations if it has multiple.
 */
export enum Context {
  /**
   * No disambiguation about a node's declarations. May be lazily generated
   * from other contexts; see SymbolVNameStore documentation.
   */
  Any,
  /** The node is declared as a getter. */
  Getter,
  /** The node is declared as a setter. */
  Setter,
}

/*
 * TODO(ayazhafiz): This is copied from
 * [typescript.d.ts](https://github.com/microsoft/TypeScript/blob/463da558a11b8fcb3ee7562a8ad9a5a41a65887f/lib/typescript.d.ts#L497).
 * We should ask if TypeScript can expose a function to determine this if this
 * type is persistently needed.
 */
type HasExpressionInitializer = ts.VariableDeclaration|
                                ts.ParameterDeclaration|ts.BindingElement|
                                ts.PropertySignature|ts.PropertyDeclaration|
                                ts.PropertyAssignment|ts.EnumMember;

/**
 * Determines if a node is a variable-like declaration.
 */
function hasExpressionInitializer(node: ts.Node):
    node is HasExpressionInitializer {
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
      return this.store.get(symbol).get(nsCtx);
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
            vnameMap.get(nsCtx).signature}`);
      }
      vnameMap.set(nsCtx, vname);
    } else {
      this.store.set(symbol, new Map([[nsCtx, vname]]));
    }

    // Set the symbol VName for the given namespace and `Any` context, if it has
    // not already been set.
    const nsAny = this.serialize(ns, Context.Any);
    vnameMap = this.store.get(symbol);
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
  private readonly symbolNames = new SymbolVNameStore();

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
      /**
       * The VName for the CompilationUnit, containing compilation-wide info.
       */
      private readonly compilationUnit: VName,
      /**
       * A map of path to path-specific VName.
       */
      private readonly pathVNames: Map<string, VName>,
      /** All source file paths in the TypeScript program. */
      public paths: string[],
      public program: ts.Program,
      private readFile: (path: string) => Buffer = fs.readFileSync,
  ) {
    this.sourceRoot = program.getCompilerOptions().rootDir || process.cwd();
    let rootDirs = program.getCompilerOptions().rootDirs || [this.sourceRoot];
    rootDirs = rootDirs.map(d => d + '/');
    rootDirs.sort((a, b) => b.length - a.length);
    this.rootDirs = rootDirs;
    this.typeChecker = this.program.getTypeChecker();
  }

  getOffsetTable(path: string): Readonly<utf8.OffsetTable> {
    let table = this.offsetTables.get(path);
    if (!table) {
      const buf = this.readFile(path);
      table = new utf8.OffsetTable(buf);
      this.offsetTables.set(path, table);
    }
    return table;
  }

  getSymbolAtLocation(node: ts.Node): ts.Symbol|undefined {
    return this.typeChecker.getSymbolAtLocation(node);
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
                  const sym = this.getSymbolAtLocation(decl.name);
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
                    !isStaticMember(lastNode, decl)) {
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
          if (!ts.isParameterPropertyDeclaration(startNode)) {
            parts.push('constructor');
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
          // it's likely this function should have handled it.  Dynamically
          // probe for this case and warn if we missed one.
          if ('name' in (node as any)) {
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
      sym: ts.Symbol, ns: TSNamespace, context: Context = Context.Any): VName {
    const stored = this.symbolNames.get(sym, ns, context);
    if (stored) return stored;

    let declarations = sym.declarations;
    if (declarations.length < 1) {
      throw new Error('TODO: symbol has no declarations?');
    }

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
    }

    // Cache the VName for future lookups.
    this.symbolNames.set(sym, ns, context, vname);
    return vname;
  }

  aliasSymbol(localSym: ts.Symbol, remoteSym: ts.Symbol): VName {
    // This imported symbol can refer to a type, a value, or both.
    // Attempt to create VNames for both the symbol's type and value and mark
    // the local symbol with the remote symbol's VName(s) so that all
    // references resolve to the remote symbol.
    let kType, kValue;
    if (remoteSym.flags & ts.SymbolFlags.Type) {
      kType = this.getSymbolName(remoteSym, TSNamespace.TYPE);
      if (kType) {
        this.symbolNames.set(localSym, TSNamespace.TYPE, Context.Any, kType);
      }
    }
    if (remoteSym.flags & ts.SymbolFlags.Value) {
      kValue = this.getSymbolName(remoteSym, TSNamespace.VALUE);
      if (kValue) {
        this.symbolNames.set(localSym, TSNamespace.VALUE, Context.Any, kValue);
      }
    }

    // The name anchor must link somewhere.  In rare cases a symbol is both
    // a type and a value that resolve to two different locations; for now,
    // because we must choose one, just prefer linking to the value.
    // One of the value or type reference should be non-null.
    return (kValue || kType);
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
   */
  pathToVName(path: string): VName {
    const vname = this.pathVNames.get(path);
    return {
      signature: '',
      language: '',
      corpus: vname && vname.corpus ? vname.corpus :
                                      this.compilationUnit.corpus,
      root: vname && vname.corpus ? vname.root : this.compilationUnit.root,
      path: vname && vname.path ? vname.path : path,
    };
  }

  /**
   * emit emits a Kythe entry, structured as a JSON object.  Defaults to
   * emitting to stdout but users may replace it.
   */
  emit = (obj: JSONFact|JSONEdge) => {
    console.log(JSON.stringify(obj));
  };
}

/** Visitor manages the indexing process for a single TypeScript SourceFile. */
class Visitor {
  /** kFile is the VName for the 'file' node representing the source file. */
  kFile: VName;

  /** A shorter name for the rootDir in the CompilerOptions. */
  sourceRoot: string;

  typeChecker: ts.TypeChecker;

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
  newAnchor(node: ts.Node, start = node.getStart(), end = node.end): VName {
    const name = Object.assign(
        {...this.kFile}, {signature: `@${start}:${end}`, language: LANGUAGE});
    this.emitNode(name, 'anchor');
    const offsetTable = this.host.getOffsetTable(node.getSourceFile().fileName);
    this.emitFact(
        name, FactName.LOC_START, offsetTable.lookupUtf8(start).toString());
    this.emitFact(
        name, FactName.LOC_END, offsetTable.lookupUtf8(end).toString());
    return name;
  }

  /** emitNode emits a new node entry, declaring the kind of a VName. */
  emitNode(source: VName, kind: string) {
    this.emitFact(source, FactName.NODE_KIND, kind);
  }

  /** emitSubkind emits a new fact entry, declaring the subkind of a VName. */
  emitSubkind(source: VName, subkind: Subkind) {
    this.emitFact(source, FactName.SUBKIND, subkind);
  }

  /** emitFact emits a new fact entry, tying an attribute to a VName. */
  emitFact(source: VName, name: FactName, value: string) {
    this.host.emit({
      source,
      fact_name: name,
      fact_value: Buffer.from(value).toString('base64'),
    });
  }

  /** emitEdge emits a new edge entry, relating two VNames. */
  emitEdge(source: VName, kind: EdgeKind|OrdinalEdge, target: VName) {
    this.host.emit({
      source,
      edge_kind: kind,
      target,
      fact_name: '/',
    });
  }

  visitTypeParameters(params: ReadonlyArray<ts.TypeParameterDeclaration>) {
    for (const param of params) {
      const sym = this.host.getSymbolAtLocation(param.name);
      if (!sym) {
        todo(
            this.sourceRoot, param,
            `type param ${param.getText()} has no symbol`);
        return;
      }
      const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
      this.emitNode(kType, 'absvar');
      this.emitEdge(
          this.newAnchor(param.name), EdgeKind.DEFINES_BINDING, kType);
      // ...<T extends A>
      if (param.constraint) {
        const superType = this.visitType(param.constraint);
        if (superType) this.emitEdge(kType, EdgeKind.BOUNDED_UPPER, superType);
      }
      // ...<T = A>
      if (param.default) this.visitType(param.default);
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
  visitHeritage(heritageClauses: ReadonlyArray<ts.HeritageClause>) {
    for (const heritage of heritageClauses) {
      if (heritage.token === ts.SyntaxKind.ExtendsKeyword && heritage.parent &&
          heritage.parent.kind !== ts.SyntaxKind.InterfaceDeclaration) {
        this.visit(heritage);
      } else {
        this.visitType(heritage);
      }
    }
  }

  visitInterfaceDeclaration(decl: ts.InterfaceDeclaration) {
    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) {
      todo(
          this.sourceRoot, decl.name,
          `interface ${decl.name.getText()} has no symbol`);
      return;
    }
    const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
    this.emitNode(kType, 'interface');
    this.emitEdge(this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, kType);

    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    if (decl.heritageClauses) this.visitHeritage(decl.heritageClauses);
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  visitTypeAliasDeclaration(decl: ts.TypeAliasDeclaration) {
    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) {
      todo(
          this.sourceRoot, decl.name,
          `type alias ${decl.name.getText()} has no symbol`);
      return;
    }
    const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
    this.emitNode(kType, 'talias');
    this.emitEdge(this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, kType);

    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
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
        const sym = this.host.getSymbolAtLocation(node);
        if (!sym) {
          todo(this.sourceRoot, node, `type ${node.getText()} has no symbol`);
          return;
        }
        const name = this.host.getSymbolName(sym, TSNamespace.TYPE);
        this.emitEdge(this.newAnchor(node), EdgeKind.REF, name);
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
    if (ts.isSourceFile(sf)) {
      return this.host.moduleName(sf.fileName);
    }
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
      const sym = this.host.getSymbolAtLocation(klass.name);
      if (sym && sym.members) {
        return sym.members.get(ts.InternalSymbolName.Constructor);
      }
    }
    return undefined;
  }

  /**
   * visitImport handles a single entry in an import statement, e.g.
   * "bar" in code like
   *   import {foo, bar} from 'baz';
   * See visitImportDeclaration for the code handling the entire statement.
   *
   * @return The VName for the import.
   */
  visitImport(name: ts.Node): VName {
    // An import both aliases a symbol from another module
    // (call it the "remote" symbol) and it defines a local symbol.
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
    // the "foo" should be both:
    // - a ref/imports to the remote symbol
    // - a defines/binding for the local symbol
    //
    // But in Kythe the UI for stacking multiple references out from a single
    // anchor isn't great, so this code instead unifies all references
    // (including renaming imports) to a single VName.

    const localSym = this.host.getSymbolAtLocation(name);
    if (!localSym) {
      throw new Error(`TODO: local name ${name} has no symbol`);
    }

    const remoteSym = this.typeChecker.getAliasedSymbol(localSym);
    const kImportName = this.host.aliasSymbol(localSym, remoteSym);
    this.emitEdge(this.newAnchor(name), EdgeKind.REF_IMPORTS, kImportName);
    return kImportName;
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
    const moduleSym = this.host.getSymbolAtLocation(moduleRef);
    if (!moduleSym) {
      // This can occur when the module failed to resolve to anything.
      // See testdata/import_missing.ts for more on how that could happen.
      return;
    }
    const modulePath = this.getModulePathFromModuleReference(moduleSym);
    if (modulePath) {
      const kModule = this.newVName('module', modulePath);
      this.emitEdge(this.newAnchor(moduleRef), EdgeKind.REF_IMPORTS, kModule);
    }

    if (ts.isImportEqualsDeclaration(decl)) {
      // This is an equals import, e.g.:
      //   import foo = require('./bar');
      this.visitImport(decl.name);
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
      this.visitImport(clause.name);
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
        this.emitNode(kModuleObject, 'variable');
        this.emitEdge(
            this.newAnchor(name), EdgeKind.DEFINES_BINDING, kModuleObject);
        break;
      case ts.SyntaxKind.NamedImports:
        // This is named imports, e.g.:
        //   import {bar, baz} from 'foo';
        const imports = clause.namedBindings.elements;
        for (const imp of imports) {
          const kImport = this.visitImport(imp.name);
          if (imp.propertyName) {
            this.emitEdge(
                this.newAnchor(imp.propertyName), EdgeKind.REF_IMPORTS,
                kImport);
          }
        }
        break;
    }
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
    this.emitFact(kMod, FactName.NODE_KIND, 'record');
    this.emitEdge(this.kFile, EdgeKind.CHILD_OF, kMod);

    // Emit the anchor, bound to the beginning of the file.
    const anchor = this.newAnchor(this.file, 0, 1);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kMod);
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

    this.emitNode(implicitProp, 'variable');
    this.emitSubkind(implicitProp, Subkind.IMPLICIT);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, implicitProp);

    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) throw new Error('Getter/setter declaration has no symbols.');

    if (sym.declarations.find(ts.isGetAccessor)) {
      // Emit a "property/reads" edge between the getter and the property
      const getter =
          this.host.getSymbolName(sym, TSNamespace.VALUE, Context.Getter);
      this.emitEdge(getter, EdgeKind.PROPERTY_READS, implicitProp);
    }
    if (sym.declarations.find(ts.isSetAccessor)) {
      // Emit a "property/writes" edge between the setter and the property
      const setter =
          this.host.getSymbolName(sym, TSNamespace.VALUE, Context.Setter);
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
    if (decl.exportClause) {
      for (const exp of decl.exportClause.elements) {
        const localSym = this.host.getSymbolAtLocation(exp.name);
        if (!localSym) {
          console.error(`TODO: export ${exp.name} has no symbol`);
          continue;
        }
        // TODO: import a type, not just a value.
        const remoteSym = this.typeChecker.getAliasedSymbol(localSym);
        const kExport = this.host.getSymbolName(remoteSym, TSNamespace.VALUE);
        this.emitEdge(this.newAnchor(exp.name), EdgeKind.REF, kExport);
        if (exp.propertyName) {
          // Aliased export; propertyName is the 'as <...>' bit.
          this.emitEdge(
              this.newAnchor(exp.propertyName), EdgeKind.REF, kExport);
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
  }): VName|undefined {
    let vname: VName|undefined;
    switch (decl.name.kind) {
      case ts.SyntaxKind.Identifier:
      case ts.SyntaxKind.ComputedPropertyName:
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
        this.emitNode(vname, 'variable');

        this.emitEdge(
            this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, vname);

        decl.name.forEachChild(child => this.visit(child));
        break;
      case ts.SyntaxKind.ObjectBindingPattern:
      case ts.SyntaxKind.ArrayBindingPattern:
        for (const element of (decl.name as ts.BindingPattern).elements) {
          this.visit(element);
        }
        break;
      default:
        break;
    }
    if (decl.type) this.visitType(decl.type);
    if (decl.initializer) this.visit(decl.initializer);
    if (vname && decl.kind === ts.SyntaxKind.PropertyDeclaration) {
      const declNode = decl as ts.PropertyDeclaration;
      if (isStaticMember(declNode, declNode.parent)) {
        this.emitFact(vname, FactName.TAG_STATIC, '');
      }
    }
    return vname;
  }

  visitFunctionLikeDeclaration(decl: ts.FunctionLikeDeclaration) {
    this.visitDecorators(decl.decorators || []);
    let kFunc: VName;
    let funcSym: ts.Symbol|undefined = undefined;
    let context: Context|undefined = undefined;
    if (ts.isGetAccessor(decl)) {
      context = Context.Getter;
    } else if (ts.isSetAccessor(decl)) {
      context = Context.Setter;
    }
    if (decl.name) {
      funcSym = this.host.getSymbolAtLocation(decl.name);
      if (decl.name.kind === ts.SyntaxKind.ComputedPropertyName) {
        this.visit((decl.name as ts.ComputedPropertyName).expression);
      }
      if (!funcSym) {
        todo(
            this.sourceRoot, decl.name,
            `function declaration ${decl.name.getText()} has no symbol`);
        return;
      }
      kFunc = this.host.getSymbolName(funcSym, TSNamespace.VALUE, context);

      const declAnchor = this.newAnchor(decl.name);
      this.emitNode(kFunc, 'function');
      this.emitEdge(declAnchor, EdgeKind.DEFINES_BINDING, kFunc);

      // Getters/setters also emit an implicit class property entry. If a
      // getter is present, it will bind this entry; otherwise a setter will.
      if (ts.isGetAccessor(decl) ||
          (ts.isSetAccessor(decl) &&
           !funcSym.declarations.find(ts.isGetAccessor))) {
        this.emitImplicitProperty(decl, declAnchor, kFunc);
      }

      this.visitJSDoc(decl, kFunc);
    } else {
      // TODO: choose VName for anonymous functions.
      kFunc = this.newVName('TODO', 'TODOPath');
    }
    this.emitEdge(this.newAnchor(decl), EdgeKind.DEFINES, kFunc);

    if (decl.parent) {
      // Emit a "childof" edge on class/interface members.
      if (ts.isClassLike(decl.parent) ||
          ts.isInterfaceDeclaration(decl.parent)) {
        const parentName = decl.parent.name;
        if (parentName !== undefined) {
          const parentSym = this.host.getSymbolAtLocation(parentName);
          if (!parentSym) {
            todo(
                this.sourceRoot, parentName,
                `parent ${parentName} has no symbol`);
            return;
          }
          const kParent = this.host.getSymbolName(parentSym, TSNamespace.TYPE);
          this.emitEdge(kFunc, EdgeKind.CHILD_OF, kParent);
        }

        // Emit "overrides" edges if this method overrides extended classes or
        // implemented interfaces, which are listed in the Heritage Clauses of
        // a class or interface.
        //     class X extends A implements B, C {}
        //             ^^^^^^^^^-^^^^^^^^^^^^^^^----- `HeritageClause`s
        // Look at each type listed in the heritage clauses and traverse its
        // members. If the type has a member that matches the method visited in
        // this function (`kFunc`), emit an "overrides" edge to that member.
        if (funcSym && decl.parent.heritageClauses) {
          for (const heritage of decl.parent.heritageClauses) {
            for (const baseType of heritage.types) {
              const baseSym =
                  this.host.getSymbolAtLocation(baseType.expression);
              if (!baseSym || !baseSym.members) {
                continue;
              }

              const funcName = funcSym.name;
              const funcFlags = funcSym.flags;

              // Find a member of with the same type (same flags) and same name
              // as the overriding method.
              const overridenCondition = (sym: ts.Symbol) =>
                  Boolean(sym.flags & funcFlags) && sym.name === funcName;

              const overriden =
                  toArray(baseSym.members.values()).find(overridenCondition);
              if (overriden) {
                this.emitEdge(
                    kFunc, EdgeKind.OVERRIDES,
                    this.host.getSymbolName(overriden, TSNamespace.VALUE));
              }
            }
          }
        }
      }
    }

    this.visitParameters(decl.parameters, kFunc);

    if (decl.type) {
      // "type" here is the return type of the function.
      this.visitType(decl.type);
    }

    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    if (decl.body) {
      this.visit(decl.body);
    } else {
      this.emitFact(kFunc, FactName.COMPLETE, 'incomplete');
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
          this.visitDecorators(param.decorators || []);

          switch (param.name.kind) {
            case ts.SyntaxKind.Identifier:
              const sym = this.host.getSymbolAtLocation(param.name);
              if (!sym) {
                todo(
                    this.sourceRoot, param.name,
                    `param ${param.name.getText()} has no symbol`);
                return;
              }
              const kParam = this.host.getSymbolName(sym, TSNamespace.VALUE);
              this.emitNode(kParam, 'variable');

              this.emitEdge(
                  kFunc, makeOrdinalEdge(EdgeKind.PARAM, paramNum), kParam);
              ++paramNum;

              if (ts.isParameterPropertyDeclaration(param)) {
                // Class members defined in the parameters of a constructor are
                // children of the class type.
                const parentName = param.parent.parent.name;
                if (parentName !== undefined) {
                  const parentSym = this.host.getSymbolAtLocation(parentName);
                  if (parentSym !== undefined) {
                    const kClass =
                        this.host.getSymbolName(parentSym, TSNamespace.TYPE);
                    this.emitEdge(kParam, EdgeKind.CHILD_OF, kClass);
                  }
                }
              } else {
                this.emitEdge(kParam, EdgeKind.CHILD_OF, kFunc);
              }

              this.emitEdge(
                  this.newAnchor(param.name), EdgeKind.DEFINES_BINDING, kParam);
              break;
            case ts.SyntaxKind.ObjectBindingPattern:
            case ts.SyntaxKind.ArrayBindingPattern:
              const elements = toArray(param.name.elements.entries());
              for (const [index, element] of elements) {
                if (ts.isBindingElement(element)) {
                  recurseVisit(element);
                }
              }
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

  visitDecorators(decors: ReadonlyArray<ts.Decorator>) {
    for (const decor of decors) {
      this.visit(decor);
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
    this.emitNode(kNamespace, 'record');
    this.emitSubkind(kNamespace, Subkind.NAMESPACE);
    const kValue = this.host.getSymbolName(sym, TSNamespace.VALUE);
    this.emitNode(kValue, 'package');

    const nameAnchor = this.newAnchor(decl.name);
    this.emitEdge(nameAnchor, EdgeKind.DEFINES_BINDING, kNamespace);
    this.emitEdge(nameAnchor, EdgeKind.DEFINES_BINDING, kValue);

    // The entire module declaration defines the created namespace.
    const defAnchor = this.newAnchor(decl);
    this.emitEdge(defAnchor, EdgeKind.DEFINES, kValue);

    if (decl.decorators) this.visitDecorators(decl.decorators);
    if (decl.body) {
      this.emitFact(kNamespace, FactName.COMPLETE, 'definition');
      this.visit(decl.body);
    } else {
      // Incomplete module definition, like
      //    declare module 'foo';
      this.emitFact(kNamespace, FactName.COMPLETE, 'incomplete');
    }
  }

  visitClassDeclaration(decl: ts.ClassDeclaration) {
    this.visitDecorators(decl.decorators || []);
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
      const kClass = this.host.getSymbolName(sym, TSNamespace.TYPE);
      this.emitNode(kClass, 'record');
      const kClassCtor = this.host.getSymbolName(sym, TSNamespace.VALUE);
      this.emitNode(kClassCtor, 'function');

      const anchor = this.newAnchor(decl.name);
      this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kClass);
      this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kClassCtor);

      // If the class has a constructor, emit an entry for it.
      const ctorSymbol = this.getCtorSymbol(decl);
      if (ctorSymbol) {
        const ctorDecl = ctorSymbol.declarations[0];
        const span = this.getTextSpan(ctorDecl, 'constructor');
        const classCtorAnchor = this.newAnchor(ctorDecl, span.start, span.end);

        const ctorVName =
            this.host.getSymbolName(ctorSymbol, TSNamespace.VALUE);

        this.emitNode(ctorVName, 'function');
        this.emitSubkind(ctorVName, Subkind.CONSTRUCTOR);
        this.emitEdge(classCtorAnchor, EdgeKind.DEFINES_BINDING, ctorVName);
      }

      this.visitJSDoc(decl, kClass);
    }
    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    if (decl.heritageClauses) this.visitHeritage(decl.heritageClauses);
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  visitEnumDeclaration(decl: ts.EnumDeclaration) {
    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) return;
    const kType = this.host.getSymbolName(sym, TSNamespace.TYPE);
    this.emitNode(kType, 'record');
    const kValue = this.host.getSymbolName(sym, TSNamespace.VALUE);
    this.emitNode(kValue, 'constant');

    const anchor = this.newAnchor(decl.name);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kType);
    this.emitEdge(anchor, EdgeKind.DEFINES_BINDING, kValue);
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  visitEnumMember(decl: ts.EnumMember) {
    const sym = this.host.getSymbolAtLocation(decl.name);
    if (!sym) return;
    const kMember = this.host.getSymbolName(sym, TSNamespace.VALUE);
    this.emitNode(kMember, 'constant');
    this.emitEdge(this.newAnchor(decl.name), EdgeKind.DEFINES_BINDING, kMember);
  }

  visitExpressionMember(node: ts.Node) {
    const sym = this.host.getSymbolAtLocation(node);
    if (!sym) {
      // E.g. a field of an "any".
      return;
    }
    if (!sym.declarations || sym.declarations.length === 0) {
      // An undeclared symbol, e.g. "undefined".
      return;
    }
    const name = this.host.getSymbolName(sym, TSNamespace.VALUE);
    const anchor = this.newAnchor(node);
    this.emitEdge(anchor, EdgeKind.REF, name);

    // Emit a 'ref/call' edge to a class constructor if a new class instance
    // is instantiated.
    if (ts.isNewExpression(node.parent)) {
      const classDecl = sym.declarations.find(ts.isClassDeclaration);
      if (classDecl) {
        const ctorSymbol = this.getCtorSymbol(classDecl);
        if (ctorSymbol) {
          const ctorVName =
              this.host.getSymbolName(ctorSymbol, TSNamespace.VALUE);
          this.emitEdge(anchor, EdgeKind.REF_CALL, ctorVName);
        }
      }
    }
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
    const thisAnchor = this.newAnchor(keyword);
    this.emitEdge(thisAnchor, EdgeKind.REF, type);
  }

  /**
   * visitJSDoc attempts to attach a 'doc' node to a given target, by looking
   * for JSDoc comments.
   */
  visitJSDoc(node: ts.Node, target: VName) {
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
    this.emitNode(doc, 'doc');
    this.emitEdge(doc, EdgeKind.DOCUMENTS, target);
    this.emitFact(doc, FactName.TEXT, jsdoc);
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
      default:
        // Use default recursive processing.
        return ts.forEachChild(node, n => this.visit(n));
    }
  }

  /** index is the main entry point, starting the recursive visit. */
  index() {
    this.emitFact(this.kFile, FactName.NODE_KIND, 'file');
    this.emitFact(this.kFile, FactName.TEXT, this.file.text);

    this.emitModuleAnchor(this.file);

    ts.forEachChild(this.file, n => this.visit(n));
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
 * @param compilationUnit A VName for the entire compilation, containing e.g.
 *     corpus name.
 * @param pathVNames A map of file path to path-specific VName.
 * @param emit If provided, a function that receives objects as they are
 *     emitted; otherwise, they are printed to stdout.
 * @param plugins If provided, a list of plugin indexers to run after the
 *     TypeScript program has been indexed.
 * @param readFile If provided, a function that reads a file as bytes to a
 *     Node Buffer.  It'd be nice to just reuse program.getSourceFile but
 *     unfortunately that returns a (Unicode) string and we need to get at
 *     each file's raw bytes for UTF-8<->UTF-16 conversions.
 */
export function index(
    vname: VName, pathVNames: Map<string, VName>, paths: string[],
    program: ts.Program, emit?: (obj: {}) => void, plugins?: Plugin[],
    readFile: (path: string) => Buffer = fs.readFileSync) {
  // Note: we only call getPreEmitDiagnostics (which causes type checking to
  // happen) on the input paths as provided in paths.  This means we don't
  // e.g. type-check the standard library unless we were explicitly told to.
  const diags = new Set<ts.Diagnostic>();
  for (const path of paths) {
    for (const diag of ts.getPreEmitDiagnostics(
             program, program.getSourceFile(path))) {
      diags.add(diag);
    }
  }
  if (diags.size > 0) {
    const message = ts.formatDiagnostics(Array.from(diags), {
      getCurrentDirectory() {
        return program.getCompilerOptions().rootDir!;
      },
      getCanonicalFileName(fileName: string) {
        return fileName;
      },
      getNewLine() {
        return '\n';
      },
    });
    throw new Error(message);
  }

  const indexingContext =
      new StandardIndexerContext(vname, pathVNames, paths, program, readFile);
  if (emit != null) {
    indexingContext.emit = emit;
  }

  for (const path of paths) {
    const sourceFile = program.getSourceFile(path);
    if (!sourceFile) {
      throw new Error(`requested indexing ${path} not found in program`);
    }
    const visitor = new Visitor(
        indexingContext,
        sourceFile,
    );
    visitor.index();
  }

  if (plugins) {
    for (const plugin of plugins) {
      try {
        plugin.index(indexingContext);
      } catch (err) {
        console.error(`Plugin ${plugin.name} errored: ${err}`);
      }
    }
  }
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
  const compilationUnit: VName = {
    corpus: 'corpus',
    root: '',
    path: '',
    signature: '',
    language: '',
  };
  const program = ts.createProgram(inPaths, config.options);
  index(compilationUnit, new Map(), inPaths, program);
  return 0;
}

if (require.main === module) {
  // Note: do not use process.exit(), because that does not ensure that
  // process.stdout has been flushed(!).
  process.exitCode = main(process.argv.slice(2));
}
