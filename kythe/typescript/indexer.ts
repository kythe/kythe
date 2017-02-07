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

import * as ts from 'typescript';

/** VName is the type of Kythe node identities. */
interface VName {
  signature: string;
  path: string;
  language: string;
  root: string;
  corpus: string;
}

/** Visitor manages the indexing process for a single TypeScript SourceFile. */
class Vistor {
  /** kFile is the VName for the source file. */
  kFile: VName;

  constructor(
      private typeChecker: ts.TypeChecker,
      /**
       * Path to the source file, for use in generated output.  Note
       * that sourceFile.sourcePath is the absolute path to the source
       * file, but for output purposes we want a repository-relative
       * path.
       */
      private sourcePath: string) {}

  /**
   * emit emits a Kythe entry, structured as a JSON object.  Defaults to
   * emitting to stdout but users may replace it.
   */
  emit =
      (obj: any) => {
        // TODO: allow control of where the output is produced.
        console.log(JSON.stringify(obj));
      }

  /** newVName returns a new VName pointing at the current file. */
  newVName(signature: string): VName {
    return {
      signature,
      path: this.sourcePath,
      language: 'typescript',
      root: '',
      corpus: 'TODO',
    };
  }

  /** newNode emits a new node entry and returns its VName. */
  newNode(signature: string, kind: string): VName {
    let vn = this.newVName(signature);
    this.emitFact(vn, 'node/kind', kind);
    return vn;
  }

  /** newAnchor emits a new anchor entry that covers a TypeScript node. */
  newAnchor(node: ts.Node): VName {
    let name = this.newNode(`@${node.pos}:${node.end}`, 'anchor');
    // TODO: loc/* should be in bytes, but these offsets are in UTF-16 units.
    this.emitFact(name, 'loc/start', node.getStart().toString());
    this.emitFact(name, 'loc/end', node.getEnd().toString());
    this.emitEdge(name, 'childof', this.kFile);
    return name;
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
      edge_kind: '/kythe/edge/' + name, target,
      fact_name: '/',
    });
  }

  /** getSymbolName computes the VName (and signature) of a ts.Symbol. */
  getSymbolName(sym: ts.Symbol): VName {
    // TODO: this is totally incorrect but it's sufficient to make the
    // current test pass, and future tests will better establish what
    // behavior is actually needed.
    return this.newVName(sym.name);
  }

  visitVariableDeclaration(decl: ts.VariableDeclaration) {
    if (decl.name.kind === ts.SyntaxKind.Identifier) {
      let kVar = this.newNode(decl.name.text, 'variable');
      let anchor = this.newAnchor(decl.name);
      this.emitEdge(anchor, 'defines/binding', kVar);
    } else {
      console.warn(
          'TODO: handle variable declaration:', ts.SyntaxKind[decl.name.kind]);
    }
  }

  /** visit is the main dispatch for visiting AST nodes. */
  visit(node: ts.Node): void {
    switch (node.kind) {
      case ts.SyntaxKind.VariableDeclaration:
        return this.visitVariableDeclaration(node as ts.VariableDeclaration);
      case ts.SyntaxKind.Identifier:
        // Assume that this identifer is occurring as part of an
        // expression; we'll handle identifiers that occur in other
        // circumstances (e.g. in a type) separately.
        let sym = this.typeChecker.getSymbolAtLocation(node);
        let name = this.getSymbolName(sym);
        this.emitEdge(this.newAnchor(node), 'ref', name);
        return;
      case ts.SyntaxKind.VariableStatement:
      case ts.SyntaxKind.VariableDeclarationList:
      case ts.SyntaxKind.ExpressionStatement:
      case ts.SyntaxKind.PostfixUnaryExpression:
      case ts.SyntaxKind.EndOfFileToken:
        // Use default recursive processing.
        return ts.forEachChild(node, n => this.visit(n));
      default:
        console.warn(`TODO: handle input node: ${ts.SyntaxKind[node.kind]}`);
    }
  }

  /** indexFile is the main entry point, starting the recursive visit. */
  indexFile(file: ts.SourceFile) {
    this.kFile = this.newVName(/* empty signature */ '');
    this.kFile.language = '';
    this.emitFact(this.kFile, 'node/kind', 'file');
    this.emitFact(this.kFile, 'text', file.text);
    ts.forEachChild(file, n => this.visit(n));
  }
}

/**
 * index indexes a TypeScript program, producing Kythe JSON objects for the
 * source file in the specified path.
 *
 * (A ts.Program is a configured collection of parsed source files, but
 * the caller must specify the source files within the program that they want
 * Kythe output for, because e.g. the standard library is contained within
 * the Program and we only want to process it once.)
 *
 * @param emit If provided, a function that receives objects as they
 */
export function index(
    path: string, program: ts.Program, emit?: (obj: any) => void) {
  let diags = ts.getPreEmitDiagnostics(program);
  if (diags.length > 0) {
    let message = ts.formatDiagnostics(diags, {
      getCurrentDirectory() {
        return process.cwd();
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
  let sourceFile = program.getSourceFile(path);
  let visitor = new Vistor(program.getTypeChecker(), path);
  if (emit != null) {
    visitor.emit = emit;
  }
  visitor.indexFile(sourceFile);
}

function main(argv: string[]) {
  if (argv.length !== 3) {
    console.error('usage: indexer path.ts');
    return 1;
  }
  let inPath = argv[2];
  let tsOpts: ts.CompilerOptions = {};
  let program = ts.createProgram([inPath], tsOpts);
  index(inPath, program);
  return 0;
}

if (require.main === module) {
  process.exit(main(process.argv));
}
