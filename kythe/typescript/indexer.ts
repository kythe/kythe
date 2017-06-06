/*
 * Copyright 2017 Google Inc. All rights reserved.
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

import * as utf8 from './utf8';

/** VName is the type of Kythe node identities. */
export interface VName {
  signature: string;
  corpus: string;
  root: string;
  path: string;
  language: string;
}

/**
 * toArray converts an Iterator to an array of its values.
 * It's necessary when running in ES5 environments where for-of loops
 * don't iterate through Iterators.
 */
function toArray<T>(it: Iterator<T>): T[] {
  let array: T[] = [];
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
 * TSNamespace represents the two namespaces of TypeScript: types and values.
 * A given symbol may be a type, it may be a value, and the two may even
 * be unrelated.
 *
 * See the table at
 *   https://www.typescriptlang.org/docs/handbook/declaration-merging.html
 *
 * TODO: there are actually three namespaces; the third is (confusingly)
 * itself called namespaces.  Implement those in this enum and other places.
 */
enum TSNamespace {
  TYPE,
  VALUE,
}

/** Visitor manages the indexing process for a single TypeScript SourceFile. */
class Vistor {
  /** kFile is the VName for the source file. */
  kFile: VName;

  /**
   * symbolNames maps ts.Symbols to their assigned VNames.
   * The value is a tuple of the separate TypeScript namespaces, and entries
   * in it correspond to TSNamespace values.  See the documentation of
   * TSNamespace.
   */
  symbolNames = new Map<ts.Symbol, [VName | null, VName|null]>();

  /**
   * anonId increments for each anonymous block, to give them unique
   * signatures.
   */
  anonId = 0;

  typeChecker: ts.TypeChecker;

  /** A shorter name for the rootDir in the CompilerOptions. */
  sourceRoot: string;

  /**
   * rootDirs is the list of rootDirs in the compiler options, sorted
   * longest first.  See this.moduleName().
   */
  rootDirs: string[];

  constructor(
      /** Corpus name for produced VNames. */
      private corpus: string, program: ts.Program,
      private getOffsetTable: (path: string) => utf8.OffsetTable) {
    this.typeChecker = program.getTypeChecker();

    this.sourceRoot = program.getCompilerOptions().rootDir || process.cwd();
    let rootDirs = program.getCompilerOptions().rootDirs || [this.sourceRoot];
    rootDirs = rootDirs.map(d => d + '/');
    rootDirs.sort((a, b) => b.length - a.length);
    this.rootDirs = rootDirs;
  }

  /**
   * emit emits a Kythe entry, structured as a JSON object.  Defaults to
   * emitting to stdout but users may replace it.
   */
  emit = (obj: {}) => {
    console.log(JSON.stringify(obj));
  };

  todo(node: ts.Node, message: string) {
    const sourceFile = node.getSourceFile();
    const file = path.relative(this.sourceRoot, sourceFile.fileName);
    const {line, character} =
        ts.getLineAndCharacterOfPosition(sourceFile, node.getStart());
    console.warn(`TODO: ${file}:${line}:${character}: ${message}`);
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
   * newVName returns a new VName with a given signature and path.
   */
  newVName(signature: string, path: string): VName {
    return {
      signature,
      corpus: this.corpus,
      root: '',
      path,
      language: 'typescript',
    };
  }

  /** newAnchor emits a new anchor entry that covers a TypeScript node. */
  newAnchor(node: ts.Node, start = node.getStart(), end = node.end): VName {
    let name = this.newVName(
        `@${start}:${end}`,
        // An anchor refers to specific text, so its path is the file path,
        // not the module name.
        path.relative(this.sourceRoot, node.getSourceFile().fileName));
    this.emitNode(name, 'anchor');
    let offsetTable = this.getOffsetTable(node.getSourceFile().fileName);
    this.emitFact(name, 'loc/start', offsetTable.lookup(start).toString());
    this.emitFact(name, 'loc/end', offsetTable.lookup(end).toString());
    this.emitEdge(name, 'childof', this.kFile);
    return name;
  }

  /** emitNode emits a new node entry, declaring the kind of a VName. */
  emitNode(source: VName, kind: string) {
    this.emitFact(source, 'node/kind', kind);
  }

  /** emitFact emits a new fact entry, tying an attribute to a VName. */
  emitFact(source: VName, name: string, value: string) {
    this.emit({
      source,
      fact_name: '/kythe/' + name,
      fact_value: new Buffer(value).toString('base64'),
    });
  }

  /** emitEdge emits a new edge entry, relating two VNames. */
  emitEdge(source: VName, name: string, target: VName) {
    this.emit({
      source,
      edge_kind: '/kythe/edge/' + name,
      target,
      fact_name: '/',
    });
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
    let parts: string[] = [];

    // Traverse the containing blocks upward, gathering names from nodes that
    // introduce scopes.
    for (let node: ts.Node|undefined = startNode; node != null;
         node = node.parent) {
      switch (node.kind) {
        case ts.SyntaxKind.ExportAssignment:
          const exportDecl = node as ts.ExportAssignment;
          if (!exportDecl.isExportEquals) {
            // It's an "export default" statement.
            // This is semantically equivalent to exporting a variable
            // named 'default'.
            parts.push('default');
          } else {
            this.todo(node, 'handle ExportAssignment with =');
          }
          break;
        case ts.SyntaxKind.ArrowFunction:
          // Arrow functions are anonymous, so generate a unique id.
          parts.push(`arrow${this.anonId++}`);
          break;
        case ts.SyntaxKind.Block:
          if (node.parent &&
              (node.parent.kind === ts.SyntaxKind.FunctionDeclaration ||
               node.parent.kind === ts.SyntaxKind.MethodDeclaration)) {
            // A block that's an immediate child of a function is the
            // function's body, so it doesn't need a separate name.
            continue;
          }
          parts.push(`block${this.anonId++}`);
          break;
        case ts.SyntaxKind.BindingElement:
        case ts.SyntaxKind.ClassDeclaration:
        case ts.SyntaxKind.FunctionDeclaration:
        case ts.SyntaxKind.InterfaceDeclaration:
        case ts.SyntaxKind.ImportSpecifier:
        case ts.SyntaxKind.ExportSpecifier:
        case ts.SyntaxKind.MethodDeclaration:
        case ts.SyntaxKind.MethodSignature:
        case ts.SyntaxKind.NamespaceImport:
        case ts.SyntaxKind.Parameter:
        case ts.SyntaxKind.PropertyAssignment:
        case ts.SyntaxKind.PropertyDeclaration:
        case ts.SyntaxKind.PropertySignature:
        case ts.SyntaxKind.TypeAliasDeclaration:
        case ts.SyntaxKind.TypeParameter:
        case ts.SyntaxKind.VariableDeclaration:
          let decl = node as ts.Declaration;
          if (decl.name && decl.name.kind === ts.SyntaxKind.Identifier) {
            parts.push(decl.name.text);
          } else {
            // TODO: handle other declarations, e.g. binding patterns.
            parts.push(`anon${this.anonId++}`);
          }
          break;
        case ts.SyntaxKind.Constructor:
          parts.push('constructor');
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
        default:
          // Most nodes are children of other nodes that do not introduce a
          // new namespace, e.g. "return x;", so ignore all other parents
          // by default.
          // TODO: namespace {}, etc.

          // If the node has a 'name' attribute it's likely this function should
          // have handled it.  Warn if not.
          if ('name' in node) {
            this.todo(
                node,
                `scopedSignature: ${ts.SyntaxKind[node.kind]} ` +
                    `has unused 'name' property`);
          }
      }
    }

    // The names were gathered from bottom to top, so reverse before joining.
    const sig = parts.reverse().join('.');
    return this.newVName(sig, moduleName!);
  }

  /**
   * getSymbolAtLocation is the same as this.typeChecker.getSymbolAtLocation,
   * except that it has a return type that properly captures that
   * getSymbolAtLocation can return undefined.  (The TypeScript API itself is
   * not yet null-safe, so it hasn't been annotated with full types.)
   */
  getSymbolAtLocation(node: ts.Node): ts.Symbol|undefined {
    return this.typeChecker.getSymbolAtLocation(node);
  }

  /** getSymbolName computes the VName (and signature) of a ts.Symbol. */
  getSymbolName(sym: ts.Symbol, ns: TSNamespace): VName {
    let vnames = this.symbolNames.get(sym);
    if (vnames && vnames[ns]) return vnames[ns]!;

    if (!sym.declarations || sym.declarations.length < 1) {
      throw new Error('TODO: symbol has no declarations?');
    }
    // TODO: think about symbols with multiple declarations.

    let decl = sym.declarations[0];
    let vname = this.scopedSignature(decl);
    // The signature of a value is undecorated;
    // the signature of a type has the #type suffix.
    if (ns === TSNamespace.TYPE) {
      vname.signature += '#type';
    }

    // Save it in the appropriate slot in the symbolNames table.
    if (!vnames) vnames = [null, null];
    vnames[ns] = vname;
    this.symbolNames.set(sym, vnames);

    return vname;
  }

  visitTypeParameters(params: ts.TypeParameterDeclaration[]) {
    for (const param of params) {
      const sym = this.getSymbolAtLocation(param.name);
      if (!sym) {
        this.todo(param, `type param ${param.getText()} has no symbol`);
        return;
      }
      const kType = this.getSymbolName(sym, TSNamespace.TYPE);
      this.emitNode(kType, 'absvar');
      this.emitEdge(this.newAnchor(param.name), 'defines/binding', kType);
    }
  }

  visitInterfaceDeclaration(decl: ts.InterfaceDeclaration) {
    let sym = this.getSymbolAtLocation(decl.name);
    if (!sym) {
      this.todo(decl.name, `interface ${decl.name.getText()} has no symbol`);
      return;
    }
    let kType = this.getSymbolName(sym, TSNamespace.TYPE);
    this.emitNode(kType, 'interface');
    this.emitEdge(this.newAnchor(decl.name), 'defines/binding', kType);

    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    if (decl.heritageClauses) {
      for (const heritage of decl.heritageClauses) {
        this.visit(heritage);
      }
    }
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  visitTypeAliasDeclaration(decl: ts.TypeAliasDeclaration) {
    let sym = this.getSymbolAtLocation(decl.name);
    if (!sym) {
      this.todo(decl.name, `type alias ${decl.name.getText()} has no symbol`);
      return;
    }
    let kType = this.getSymbolName(sym, TSNamespace.TYPE);
    this.emitNode(kType, 'talias');
    this.emitEdge(this.newAnchor(decl.name), 'defines/binding', kType);

    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    this.visitType(decl.type);
    // TODO: in principle right here we emit an "aliases" edge.
    // However, it's complicated by the fact that some types don't have
    // specific names to alias, e.g.
    //   type foo = number|string;
    // Just punt for now.
  }

  /**
   * visitType is the main dispatch for visiting type nodes.
   * It's separate from visit() because bare ts.Identifiers within a normal
   * expression are values (handled by visit) but bare ts.Identifiers within
   * a type are types (handled here).
   */
  visitType(node: ts.Node): void {
    switch (node.kind) {
      case ts.SyntaxKind.Identifier:
        let sym = this.getSymbolAtLocation(node);
        if (!sym) {
          this.todo(node, `type ${node.getText()} has no symbol`);
          return;
        }
        let name = this.getSymbolName(sym, TSNamespace.TYPE);
        this.emitEdge(this.newAnchor(node), 'ref', name);
        return;
      default:
        // Default recursion, but using visitType(), not visit().
        return ts.forEachChild(node, n => this.visitType(n));
    }
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
  getModulePathFromModuleReference(sym: ts.Symbol): string {
    let name = sym.name;
    // TODO: this is hacky; it may be the case we need to use the TypeScript
    // module resolver to get the real path (?).  But it appears the symbol
    // name is the quoted(!) path to the module.
    if (!(name.startsWith('"') && name.endsWith('"'))) {
      throw new Error(`TODO: handle module symbol ${name}`);
    }
    const sourcePath = name.substr(1, name.length - 2);
    return this.moduleName(sourcePath);
  }

  /**
   * visitImportSpecifier handles a single entry in an import statement, e.g.
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

    const localSym = this.getSymbolAtLocation(name);
    if (!localSym) {
      throw new Error(`TODO: local name ${name} has no symbol`);
    }

    // TODO: import a type, not just a value.
    const remoteSym = this.typeChecker.getAliasedSymbol(localSym);
    const kImport = this.getSymbolName(remoteSym, TSNamespace.VALUE);
    // Mark the local symbol with the remote symbol's VName so that all
    // references resolve to the remote symbol.
    this.symbolNames.set(localSym, [null, kImport]);

    this.emitEdge(this.newAnchor(name), 'ref/imports', kImport);
    return kImport;
  }

  /** visitImportDeclaration handles the various forms of "import ...". */
  visitImportDeclaration(decl: ts.ImportDeclaration) {
    // All varieties of import statements reference a module on the right,
    // so start by linking that.
    let moduleSym = this.getSymbolAtLocation(decl.moduleSpecifier);
    if (!moduleSym) {
      this.todo(decl.moduleSpecifier, `import ${decl.getText()} has no symbol`);
      return;
    }
    let kModule = this.newVName(
        'module', this.getModulePathFromModuleReference(moduleSym));
    this.emitEdge(this.newAnchor(decl.moduleSpecifier), 'ref/imports', kModule);

    if (!decl.importClause) {
      // This is a side-effecting import that doesn't declare anything, e.g.:
      //   import 'foo';
      return;
    }
    let clause = decl.importClause;

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
        let name = clause.namedBindings.name;
        let sym = this.getSymbolAtLocation(name);
        if (!sym) {
          this.todo(
              clause, `namespace import ${clause.getText()} has no symbol`);
          return;
        }
        let kModuleObject = this.getSymbolName(sym, TSNamespace.VALUE);
        this.emitNode(kModuleObject, 'variable');
        this.emitEdge(this.newAnchor(name), 'defines/binding', kModuleObject);
        break;
      case ts.SyntaxKind.NamedImports:
        // This is named imports, e.g.:
        //   import {bar, baz} from 'foo';
        let imports = clause.namedBindings.elements;
        for (let imp of imports) {
          const kImport = this.visitImport(imp.name);
          if (imp.propertyName) {
            this.emitEdge(
                this.newAnchor(imp.propertyName), 'ref/imports', kImport);
          }
        }
        break;
    }
  }

  /**
   * Handles code like:
   *   export default ...;
   *   export = ...;
   */
  visitExportAssignment(assign: ts.ExportAssignment) {
    if (assign.isExportEquals) {
      this.todo(assign, `handle export = statement`);
    } else {
      // export default <expr>;
      // is the same as exporting the expression under the symbol named
      // "default".  But we don't have a nice name to link the symbol to!
      // So instead we link the keyword "default" itself to the VName.
      // The TypeScript AST does not expose the location of the 'default'
      // keyword so we just find it in the source text to link it.
      const ofs = assign.getText().indexOf('default');
      if (ofs < 0) throw new Error(`'export default' without 'default'?`);
      const start = assign.getStart() + ofs;
      const anchor = this.newAnchor(assign, start, start + 'default'.length);
      this.emitEdge(anchor, 'defines/binding', this.scopedSignature(assign));
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
        const localSym = this.getSymbolAtLocation(exp.name);
        if (!localSym) {
          console.error(`TODO: export ${name} has no symbol`);
          continue;
        }
        // TODO: import a type, not just a value.
        const remoteSym = this.typeChecker.getAliasedSymbol(localSym);
        const kExport = this.getSymbolName(remoteSym, TSNamespace.VALUE);
        this.emitEdge(this.newAnchor(exp.name), 'ref', kExport);
        if (exp.propertyName) {
          // Aliased export; propertyName is the 'as <...>' bit.
          this.emitEdge(this.newAnchor(exp.propertyName), 'ref', kExport);
        }
      }
    }
    if (decl.moduleSpecifier) {
      this.todo(
          decl.moduleSpecifier, `handle module specifier in ${decl.getText()}`);
    }
  }

  /**
   * Note: visitVariableDeclaration is also used for class properties;
   * the decl parameter is the union of the attributes of the two types.
   */
  visitVariableDeclaration(decl: {
    name: ts.BindingName | ts.PropertyName,
    type?: ts.TypeNode,
    initializer?: ts.Expression,
  }) {
    switch (decl.name.kind) {
      case ts.SyntaxKind.Identifier:
        let sym = this.getSymbolAtLocation(decl.name);
        if (!sym) {
          this.todo(
              decl.name, `declaration ${decl.name.getText()} has no symbol`);
          return;
        }
        let kVar = this.getSymbolName(sym, TSNamespace.VALUE);
        this.emitNode(kVar, 'variable');

        this.emitEdge(this.newAnchor(decl.name), 'defines/binding', kVar);
        break;
      case ts.SyntaxKind.ObjectBindingPattern:
      case ts.SyntaxKind.ArrayBindingPattern:
        for (const element of (decl.name as ts.BindingPattern).elements) {
          this.visit(element);
        }
        break;
      default:
        this.todo(
            decl.name,
            `handle variable declaration: ${ts.SyntaxKind[decl.name.kind]}`);
    }
    if (decl.type) this.visitType(decl.type);
    if (decl.initializer) this.visit(decl.initializer);
  }

  visitFunctionLikeDeclaration(decl: ts.FunctionLikeDeclaration) {
    let kFunc: VName|undefined = undefined;
    if (decl.name) {
      let sym = this.getSymbolAtLocation(decl.name);
      if (decl.name.kind === ts.SyntaxKind.ComputedPropertyName) {
        // TODO: it's not clear what to do with computed property named
        // functions.  They don't have a symbol.
        this.visit((decl.name as ts.ComputedPropertyName).expression);
      } else {
        if (!sym) {
          this.todo(
              decl.name,
              `function declaration ${decl.name.getText()} has no symbol`);
          return;
        }
        kFunc = this.getSymbolName(sym, TSNamespace.VALUE);
        this.emitNode(kFunc, 'function');

        this.emitEdge(this.newAnchor(decl.name), 'defines/binding', kFunc);
      }
    } else {
      // TODO: choose VName for anonymous functions.
      kFunc = this.newVName('TODO', 'TODOPath');
    }

    if (kFunc && decl.parent) {
      // Emit a "childof" edge on class/interface members.
      let parentName: ts.Identifier|undefined;
      let namespace: TSNamespace|undefined;
      switch (decl.parent.kind) {
        case ts.SyntaxKind.ClassDeclaration:
        case ts.SyntaxKind.ClassExpression:
          parentName = (decl.parent as ts.ClassLikeDeclaration).name;
          namespace = TSNamespace.VALUE;
          break;
        case ts.SyntaxKind.InterfaceDeclaration:
          parentName = (decl.parent as ts.InterfaceDeclaration).name;
          namespace = TSNamespace.TYPE;
          break;
      }
      if (parentName !== undefined && namespace !== undefined) {
        let parentSym = this.getSymbolAtLocation(parentName);
        if (!parentSym) {
          this.todo(parentName, `parent ${parentName} has no symbol`);
          return;
        }
        let kParent = this.getSymbolName(parentSym, namespace);
        this.emitEdge(kFunc, 'childof', kParent);
      }

      // TODO: emit an "overrides" edge if this method overrides.
      // It appears the TS API doesn't make finding that information easy,
      // perhaps because it mostly cares about whether types are structrually
      // compatible.  But I think you can start from the parent class/iface,
      // then from there follow the "implements"/"extends" chain to other
      // classes/ifaces, and then from there look for methods with matching
      // names.
    }

    for (const [index, param] of toArray(decl.parameters.entries())) {
      let sym = this.getSymbolAtLocation(param.name);
      if (!sym) {
        this.todo(param.name, `param ${param.name.getText()} has no symbol`);
        continue;
      }
      let kParam = this.getSymbolName(sym, TSNamespace.VALUE);
      this.emitNode(kParam, 'variable');
      if (kFunc) this.emitEdge(kFunc, `param.${index}`, kParam);

      this.emitEdge(this.newAnchor(param.name), 'defines/binding', kParam);
      if (param.type) this.visitType(param.type);
    }

    if (decl.type) {
      // "type" here is the return type of the function.
      this.visitType(decl.type);
    }

    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    if (decl.body) {
      this.visit(decl.body);
    } else {
      if (kFunc) this.emitFact(kFunc, 'complete', 'incomplete');
    }
  }

  visitClassDeclaration(decl: ts.ClassDeclaration) {
    if (decl.name) {
      let sym = this.getSymbolAtLocation(decl.name);
      if (!sym) {
        this.todo(decl.name, `class ${decl.name.getText()} has no symbol`);
        return;
      }
      let kClass = this.getSymbolName(sym, TSNamespace.VALUE);
      this.emitNode(kClass, 'record');

      this.emitEdge(this.newAnchor(decl.name), 'defines/binding', kClass);
    }
    if (decl.typeParameters) this.visitTypeParameters(decl.typeParameters);
    if (decl.heritageClauses) {
      for (const heritage of decl.heritageClauses) {
        this.visit(heritage);
      }
    }
    for (const member of decl.members) {
      this.visit(member);
    }
  }

  /** visit is the main dispatch for visiting AST nodes. */
  visit(node: ts.Node): void {
    switch (node.kind) {
      case ts.SyntaxKind.ImportDeclaration:
        return this.visitImportDeclaration(node as ts.ImportDeclaration);
      case ts.SyntaxKind.ExportAssignment:
        return this.visitExportAssignment(node as ts.ExportAssignment);
      case ts.SyntaxKind.ExportDeclaration:
        return this.visitExportDeclaration(node as ts.ExportDeclaration);
      case ts.SyntaxKind.VariableDeclaration:
        return this.visitVariableDeclaration(node as ts.VariableDeclaration);
      case ts.SyntaxKind.PropertyDeclaration:
      case ts.SyntaxKind.PropertySignature:
        return this.visitVariableDeclaration(node as ts.PropertyDeclaration);
      case ts.SyntaxKind.ArrowFunction:
      case ts.SyntaxKind.Constructor:
      case ts.SyntaxKind.FunctionDeclaration:
      case ts.SyntaxKind.MethodDeclaration:
      case ts.SyntaxKind.MethodSignature:
        return this.visitFunctionLikeDeclaration(
            node as ts.FunctionLikeDeclaration);
      case ts.SyntaxKind.ClassDeclaration:
        return this.visitClassDeclaration(node as ts.ClassDeclaration);
      case ts.SyntaxKind.InterfaceDeclaration:
        return this.visitInterfaceDeclaration(node as ts.InterfaceDeclaration);
      case ts.SyntaxKind.TypeAliasDeclaration:
        return this.visitTypeAliasDeclaration(node as ts.TypeAliasDeclaration);
      case ts.SyntaxKind.TypeReference:
        return this.visitType(node as ts.TypeNode);
      case ts.SyntaxKind.BindingElement:
        return this.visitVariableDeclaration(node as ts.BindingElement);
      case ts.SyntaxKind.Identifier:
        // Assume that this identifer is occurring as part of an
        // expression; we handle identifiers that occur in other
        // circumstances (e.g. in a type) separately in visitType.
        let sym = this.getSymbolAtLocation(node);
        if (!sym) {
          // E.g. a field of an "any".
          return;
        }
        if (!sym.declarations || sym.declarations.length === 0) {
          // An undeclared symbol, e.g. "undefined".
          return;
        }
        let name = this.getSymbolName(sym, TSNamespace.VALUE);
        this.emitEdge(this.newAnchor(node), 'ref', name);
        return;
      default:
        // Use default recursive processing.
        return ts.forEachChild(node, n => this.visit(n));
    }
  }

  /** indexFile is the main entry point, starting the recursive visit. */
  indexFile(file: ts.SourceFile) {
    // Emit a "file" node, containing the source text.
    this.kFile = this.newVName(
        /* empty signature */ '',
        path.relative(this.sourceRoot, file.fileName));
    this.kFile.language = '';
    this.emitFact(this.kFile, 'node/kind', 'file');
    this.emitFact(this.kFile, 'text', file.text);

    // Emit a "record" node, representing the module object.
    let kMod = this.newVName('module', this.moduleName(file.fileName));
    this.emitFact(kMod, 'node/kind', 'record');
    this.emitEdge(this.kFile, 'childof', kMod);

    ts.forEachChild(file, n => this.visit(n));
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
 * @param emit If provided, a function that receives objects as they are
 *     emitted; otherwise, they are printed to stdout.
 * @param readFile If provided, a function that reads a file as bytes to a
 *     Node Buffer.  It'd be nice to just reuse program.getSourceFile but
 *     unfortunately that returns a (Unicode) string and we need to get at
 *     each file's raw bytes for UTF-8<->UTF-16 conversions.
 */
export function index(
    corpus: string, paths: string[], program: ts.Program,
    emit?: (obj: {}) => void,
    readFile: (path: string) => Buffer = fs.readFileSync) {
  // Note: we only call getPreEmitDiagnostics (which causes type checking to
  // happen) on the input paths as provided in paths.  This means we don't
  // e.g. type-check the standard library unless we were explicitly told to.
  let diags = new Set<ts.Diagnostic>();
  for (const path of paths) {
    for (const diag of ts.getPreEmitDiagnostics(
             program, program.getSourceFile(path))) {
      diags.add(diag);
    }
  }
  if (diags.size > 0) {
    let message = ts.formatDiagnostics(Array.from(diags), {
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

  let offsetTables = new Map<string, utf8.OffsetTable>();
  const getOffsetTable =
      (path: string): utf8.OffsetTable => {
        let table = offsetTables.get(path);
        if (!table) {
          let buf = readFile(path);
          table = new utf8.OffsetTable(buf);
          offsetTables.set(path, table);
        }
        return table;
      }

  for (const path of paths) {
    let sourceFile = program.getSourceFile(path);
    let visitor = new Vistor(corpus, program, getOffsetTable);
    if (emit != null) {
      visitor.emit = emit;
    }
    visitor.indexFile(sourceFile);
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
  let {config: json, error} = ts.readConfigFile(tsconfigPath, host.readFile);
  if (error) {
    throw new Error(ts.formatDiagnostics([error], ts.createCompilerHost({})));
  }
  let config = ts.parseJsonConfigFileContent(json, host, projectPath);
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

  let config = loadTsConfig(argv[0], path.dirname(argv[0]));
  let inPaths = argv.slice(1);
  if (inPaths.length === 0) {
    inPaths = config.fileNames;
  }

  let program = ts.createProgram(inPaths, config.options);
  index('TODOcorpus', inPaths, program);
  return 0;
}

if (require.main === module) {
  // Note: do not use process.exit(), because that does not ensure that
  // process.stdout has been flushed(!).
  process.exitCode = main(process.argv.slice(2));
}
