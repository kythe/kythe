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

/**
 * @fileoverview Post processing that takes data produced by the indexer and
 * modifies it to handle cases which can't be effectively or easily handled
 * during normal indexing. Code here works only with Kythe edges and facts and
 * doesn't use TS AST.
 */


import {EdgeKind, JSONEdge, KytheData, VName} from './kythe';

/**
 * Convert VName to a string that can be used as key in Maps.
 */
function vnameToString(vname: VName): string {
    return `(${vname.corpus},${vname.language},${vname.path},${vname.root},${
        vname.signature})`;
  }

/**
 * Finds node pairs that represent imported nodes. Key - string representation of a vname of the
 * local node e.g. foo in `import {foo} from './bar';` and value is VName of the original node
 * e.g. `foo` in `export const foo = 1;`.
 *
 * All edges that point to the first node (local) will be reassigned to second (original).
 */
function findImportNodesThatNeedEdgesReassignment(data: Readonly<KytheData>): Map<string, VName> {
    const result = new Map<string, VName>();
    // Map of anchor ==ref/import==> node.
    const importRefs = new Map<string, VName>();
    // Map of anchor ==defines/binding==> node.
    const defines = new Map<string, VName>();
    for (const entry of data) {
      if (!('target' in entry)) continue;
      if (entry.edge_kind === EdgeKind.DEFINES_BINDING) {
        defines.set(vnameToString(entry.source), entry.target);
      }
      if (entry.edge_kind === EdgeKind.REF_IMPORTS) {
        importRefs.set(vnameToString(entry.source), entry.target);
      }
    }
    for (const [anchor, node] of importRefs.entries()) {
      const define = defines.get(anchor);
      if (define != null) {
        result.set(vnameToString(define), node);
      }
    }
    return result;

}

/**
 * This method merges nodes for imported symbols.
 *
 * Indexer produces the following graph:
 *
 * // foo.ts
 * //- @ANSWER defines/binding AnswerOrig
 * export const ANSWER = 42;
 *
 * // bar.ts
 * //- @ANSWER ref/imports AnswerOrig
 * //- @ANSWER defines/binding AnswerLocal
 * import {ANSWER} from './foo';
 * //- @ANSWER ref AnswerLocal
 * console.log(ANSWER);
 *
 *
 * This pass changes
 *
 * //- @ANSWER ref AnswerLocal
 * console.log(ANSWER);
 *
 * to
 *
 * //- @ANSWER ref AnswerOrig
 * console.log(ANSWER);
 *
 *
 * Notice that ANSWER anchor in console.log call now points at the AnswerOrig node.
 * This is what users expect: that usages of imported symbols will point to the
 * original definition and not local aliases.
 */
function reassignEdgesForImports(data: Readonly<KytheData>): KytheData {
    const result: KytheData = [];
    const nodesToReassign = findImportNodesThatNeedEdgesReassignment(data);
    for (const entry of data) {
        if ('fact_value' in entry) {
            // Add all facts as it is. We only update edges.
            result.push(entry);
            continue;
        }
        const newTargetNode = nodesToReassign.get(vnameToString(entry.target));
        // Reassign all edges except for defines. We still want the local symbol to
        // be clickable.
        if (newTargetNode &&
            entry.edge_kind !== EdgeKind.DEFINES_BINDING &&
            entry.edge_kind !== EdgeKind.DEFINES) {
            entry.target = newTargetNode;
        }
        result.push(entry);
    }
    return result;
}

/**
 * Main function of this module. Runs one or more post-processing steps to clean
 * up data.
 */
export function performPostProcessing(data: Readonly<KytheData>): KytheData {
    return reassignEdgesForImports(data);
}
