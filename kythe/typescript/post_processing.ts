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
 * Pass which inlines the following pattern:
 *
 * AnchorA ==ref==> NodeA
 * AnchorB ==defines/binding==> NodeA
 * AnchorB ==ref==> NodeB
 *
 * It will remove NodeA and all edges that point to NodeA will point to NodeB.
 * So in this case final data will be:
 *
 * AnchorA ==ref==> NodeB
 * AnchorB ==ref==> NodeB
 *
 * It will also remove all facts related to NodeA.
 */
function mergeAliasLikeNodes(data: Readonly<KytheData>): KytheData {
  // Map of anchors to nodes that anchor refers to.
  const refs = new Map<string, VName>();
  // Map of anchors to nodes that anchor defines.
  const defines = new Map<string, VName>();
  for (const entry of data) {
    if (!('target' in entry)) continue;
    if (entry.edge_kind === EdgeKind.DEFINES_BINDING) {
      defines.set(vnameToString(entry.source), entry.target);
    }
    if (entry.edge_kind === EdgeKind.REF) {
      refs.set(vnameToString(entry.source), entry.target);
    }
  }
  // Find all nodes which are defined at a location, which is also
  // a reference to another node. Such nodes will be removed.
  const nodesToMerge = new Map<string, VName>();
  for (const [anchor, node] of refs.entries()) {
    const define = defines.get(anchor);
    if (define != null) {
      nodesToMerge.set(vnameToString(define), node);
    }
  }
  const result: KytheData = [];
  for (const entry of data) {
    if ('fact_value' in entry) {
      // Only keep facts for nodes which are not going to be removed.
      if (!nodesToMerge.has(vnameToString(entry.source))) {
        result.push(entry);
      }
      continue;
    }
    const mergeTo = nodesToMerge.get(vnameToString(entry.target));
    // Skip edges where target is not a node to be removed.
    if (mergeTo == null) {
      result.push(entry);
      continue;
    }

    // Delete all `define` and `defines/binding` edges that point to the node
    // being removed. All other edges are updated to contain replacement node as
    // target.
    if (entry.edge_kind !== EdgeKind.DEFINES_BINDING &&
        entry.edge_kind !== EdgeKind.DEFINES) {
      entry.target = mergeTo;
      result.push(entry);
      continue;
    }
  }
  return result;
}

/**
 * Main function of this module. Runs one or more post-processing steps to clean
 * up data.
 */
export function runPostProcess(data: Readonly<KytheData>): KytheData {
  return mergeAliasLikeNodes(data);
}