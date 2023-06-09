/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

/**
 * @fileoverview This file contains interfaces and constants necessary to
 * implement a plugin for TS indexing.
 */

import * as ts from 'typescript';

import {JSONEdge, JSONFact, VName} from './kythe';
import * as utf8 from './utf8';

/**
 * A unit of code that can be indexed. It resembles Kythe's CompilationUnit
 * proto.
 */
export interface CompilationUnit {
  /**
   * A VName for the entire compilation, containing e.g. corpus name. Used as
   * basis for all vnames emitted by indexer.
   */
  rootVName: VName;

  /** A map of file path to file-specific VName. */
  fileVNames: Map<string, VName>;

  /** Files to index. */
  srcs: string[];

  /**
   * List of files from which TS compiler will start type checking. It should
   * include srcs files + file that provide global types, that are not imported
   * from srcs files. All other dep files must be loaded following imports and
   * by IndexingOptions.readFile function.
   */
  rootFiles: string[];
}

/**
 * Various options that are required in order to perform indexing.
 */
export interface IndexingOptions {
  /** Compiler options to use for indexing. */
  compilerOptions: ts.CompilerOptions;

  /**
   * Compiler host to use for indexing. Simplest approach is to create
   * ts.createCompilerHost(options). In more advanced cases callers
   * might augment host to cache commonly used libraries for example.
   */
  compilerHost: ts.CompilerHost;

  /** Function that receives final kythe indexing data. */
  emit: (obj: JSONFact|JSONEdge) => void;

  /**
   * If provided, a list of plugin indexers to run after the TypeScript program
   * has been indexed.
   */
  plugins?: Plugin[];

  /**
   * If provided, a function that reads a file as bytes to a Node Buffer. It'd
   * be nice to just reuse program.getSourceFile but unfortunately that returns
   * a (Unicode) string and we need to get at each file's raw bytes for
   * UTF-8<->UTF-16 conversions. If omitted - fs.readFileSync is used.
   */
  readFile?: (path: string) => Buffer;

  /**
   * When enabled emits 0-0 spans at the beginning of each file that represent
   * current module. By default 0-1 spans are emitted. Also this flag changes it
   * to emit `defines/implicit` edges instead of `defines/binding`.
   */
  emitZeroWidthSpansForModuleNodes?: boolean;

  /**
   * When enabled, ref/call source anchors span identifiers instead of full
   * call expressions when possible.
   */
  emitRefCallOverIdentifier?: boolean;
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
 *
 * TYPE_MIGRATION is a temporary namespace to be used during the tvar migration.
 */
export enum TSNamespace {
  TYPE,
  VALUE,
  NAMESPACE,
  TYPE_MIGRATION,
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

/**
 * An indexer host holds information about the program indexing and methods
 * used by the TypeScript indexer that may also be useful to plugins, reducing
 * code duplication.
 */
export interface IndexerHost {
  /** Compilation unit being indexed. */
  compilationUnit: CompilationUnit;

  options: IndexingOptions;

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
   * Similar to getSymbolAtLocation() if returned symbol is an alias. One
   * example is imports:
   *
   * // a.ts
   * export value = 42;  // #1
   *
   * // b.ts
   * import {value} from './a'; // #2
   * console.log(value);  // #3
   *
   * getSymbolAtLocationFollowingAliases for #3 will return #1 while regular
   * getSymbolAtLocation returns #2.
   */
  getSymbolAtLocationFollowingAliases(node: ts.Node): ts.Symbol|undefined;

  /**
   * Computes the VName (and signature) of a ts.Symbol. A Context can be
   * optionally specified to help disambiguate nodes with multiple declarations.
   * See the documentation of Context for more information.
   */
  getSymbolName(sym: ts.Symbol, ns: TSNamespace, context?: Context): VName
      |undefined;
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
   * TypeScript program.
   */
  program: ts.Program;
}

/**
 * A indexer plugin adds extra functionality with the same inputs as the base
 * indexer.
 */
export interface Plugin {
  /** Name of the plugin. It will be printed to stderr when running plugin. */
  name: string;
  /**
   * Indexes a TypeScript program with extra functionality.
   * Takes a indexer host, which provides useful properties and methods that
   * the plugin can defer to rather than reimplementing.
   */
  index(context: IndexerHost): void;
}
