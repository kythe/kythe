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

export function paramEdge(source : VName, target : VName, index : number)
    : Entry {
  return { source: source, target: target, edge_kind: "/kythe/edge/param",
      fact_name: "/kythe/ordinal", fact_value: "" + index }
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

var exportedAnchors : {[index:string]:boolean} = {};

export function anchor(fileName : VName, begin : number, end : number) : VName {
  var anchorVName = copyVName(fileName);
  anchorVName.signature = begin + "," + end;
  anchorVName.language = "ts";
  let anchorString = JSON.stringify(anchorVName);
  if (!exportedAnchors[anchorString]) {
    write(fact(anchorVName, "node/kind", "anchor"));
    write(fact(anchorVName, "loc/start", "" + begin));
    write(fact(anchorVName, "loc/end", "" + end));
    write(edge(anchorVName, "childof", fileName));
    exportedAnchors[anchorString] = true;
  }
  return anchorVName;
}

var exportedNames : {[index:string]:boolean} = {};

export function name(path : string[], tag : string) : VName {
  let signature = path.reverse().join(":") + "#" + tag;
  let nameVName = vname(signature, "", "ts", "", "");
  if (!exportedNames[signature]) {
    exportedNames[signature] = true;
    write(fact(nameVName, "node/kind", "name"));
  }
  return nameVName;
}

export function nameFromQualifiedName(qualified : string,
    tag : string) : VName {
  return name(qualified.split("."), tag);
}

// A rule from a vnames.json file.
interface ClassifierRule {
  pattern : RegExp;
  rootTemplate : (string | number)[];
  corpusTemplate : (string | number)[];
  pathTemplate : (string | number)[];
}

// Matches a single "text@sub-ref@" pair.
let subMatcher : RegExp = /([^@]*)@([^@]+)@/g;

// Parses a template string into an array of template actions
// (strings for string pastes; numbers for capture references)
function parseTemplate(template : string) : (string | number)[] {
  if (!template) {
    return [];
  }
  var outArray : (string | number)[] = [];
  var matchResult = subMatcher.exec(template);
  var lastIndex = 0;
  while (matchResult !== null) {
    lastIndex = subMatcher.lastIndex;
    outArray.push(matchResult[1]);
    outArray.push(Number(matchResult[2]));
    matchResult = subMatcher.exec(template);
  }
  outArray.push(template.substr(lastIndex));
  return outArray;
}

// Applies a template string using an array of capture references.
function applyTemplate(template : (string | number)[], subs : string[])
    : string {
  var result : string = "";
  for (var v = 0; v < template.length; ++v) {
    if (typeof template[v] === 'number') {
      let index = <number>(template[v]);
      if (index > 0 && index < subs.length) {
        result += subs[index];
      } else {
        result += "@" + index + "@";
      }
    } else {
      result += template[v];
    }
  }
  return result;
}

// The type of an unparsed vnames.json record.
export interface VNamesConfigFileEntry {
  pattern : string;
  vname : VName;
}

// Maps paths to VNames using vnames.json data.
export class PathClassifier {
  constructor(config: VNamesConfigFileEntry[]) {
    this.rules = [];
    for (var i = 0; i < config.length; ++i) {
      this.rules.push({
        pattern: new RegExp(config[i].pattern),
        rootTemplate: parseTemplate(config[i].vname.root),
        corpusTemplate: parseTemplate(config[i].vname.corpus),
        pathTemplate: parseTemplate(config[i].vname.path)
      });
    }
  }
  classify(path : string) : VName {
    for (var i = 0; i < this.rules.length; ++i) {
      var matches = this.rules[i].pattern.exec(path);
      if (matches != null) {
        return vname("", applyTemplate(this.rules[i].pathTemplate, matches),
            "", applyTemplate(this.rules[i].rootTemplate, matches),
            applyTemplate(this.rules[i].corpusTemplate, matches));
      }
    }
    return vname("", path, "", "", "no_corpus");
  }
  private rules : ClassifierRule[];
}
