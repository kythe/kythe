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

  /** emit emits a Kythe entry, structured as a JSON object. */
  emit(obj: any) {
    // TODO: allow control of where the output is produced.
    console.log(JSON.stringify(obj));
  }

  /** newVName returns a new VName pointing at the current file. */
  newVName(signature: string): VName {
    let corpus = 'TODO';
    return {
      signature,
      path: this.sourcePath,
      language: 'typescript',
      root: '',
      corpus,
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

  /** visit is the main dispatch for visiting AST nodes. */
  visit(node: ts.Node) {
    switch (node.kind) {
      default:
        console.warn(`TODO: handle input node: ${ts.SyntaxKind[node.kind]}`);
        ts.forEachChild(node, n => this.visit(n));
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

function main() {
  let inPath = 'indexer.ts';
  let tsOpts: ts.CompilerOptions = {};
  let program = ts.createProgram([inPath], tsOpts);
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
    console.error('error:', message);
    return 1;
  }
  let sourceFile = program.getSourceFile(inPath);

  new Vistor(program.getTypeChecker(), inPath).indexFile(sourceFile);
  return 0;
}

process.exit(main());
