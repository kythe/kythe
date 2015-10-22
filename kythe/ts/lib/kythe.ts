/*
 * Copyright 2015 Google Inc. All rights reserved.
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

export interface VName {
  signature : string;
  path : string;
  language : string;
  root : string;
  corpus : string;
}

export interface Entry {
  source : VName;
  target? : VName;
  edge_kind? : string;
  fact_name : string;
  fact_value : string;
}

export function vname(signature : string,
               path : string,
               language : string,
               root : string,
               corpus : string) : VName {
  return { signature: signature, path: path, language: language,
      root: root, corpus: corpus };
}

export function fact(node : VName, factName : string, factVal : string)
    : Entry {
  return { source: node, fact_name: "/kythe/" + factName, fact_value: factVal };
}

export function edge(source : VName, edgeKind : string, target : VName)
    : Entry {
  return { source: source, target: target, edge_kind: "/kythe/edge/" + edgeKind,
      fact_name: "/", fact_value: "" }
}

export function write(entry : Entry) : void {
  var oldFactVal = entry.fact_value;
  entry.fact_value = (new Buffer(oldFactVal)).toString('base64');
  process.stdout.write(JSON.stringify(entry));
  entry.fact_value = oldFactVal;
}

export function copyVName(v : VName) : VName {
  return vname(v.signature, v.path, v.language, v.root, v.corpus);
}

export function anchor(fileName : VName, begin : number, end : number) : VName {
  var anchorVName = copyVName(fileName);
  anchorVName.signature = begin + "," + end;
  write(fact(anchorVName, "node/kind", "anchor"));
  write(fact(anchorVName, "loc/start", "" + begin));
  write(fact(anchorVName, "loc/end", "" + end));
  write(edge(anchorVName, "childof", fileName));
  return anchorVName;
}
