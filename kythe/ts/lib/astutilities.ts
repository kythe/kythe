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

import ts = require('typescript');
import tsu = require('./tsutilities');
import jobs = require('./job');

/// Heuristic function to determine whether we consider an `exports` variable
/// defined by `name` to be the canonical `exports` for the module system.
function fileNameCanProvideExportIdentifier(name : string) : boolean {
  return name.match(/\/node.d.ts$/) != null;
}

export function tokenIsExports(lang : ts.LanguageService, job : jobs.Job,
                               token : ts.Node) : boolean {
  if (token.kind == ts.SyntaxKind.Identifier) {
    let ident = <ts.Identifier>token;
    if (ident.text != 'exports') {
      return false;
    }
    let sourceFile = tsu.getSourceFileOfNode(token);
    let defs =
        lang.getDefinitionAtPosition(sourceFile.fileName, ident.getStart());
    return defs && defs.length == 1
           && (defs[0].kind == 'var' || defs[0].kind == 'property')
           && fileNameCanProvideExportIdentifier(defs[0].fileName);
  }
  return false;
}

export interface ExportInfo {
  /// The name being bound. If null, this is an assignment directly to the
  /// module (exports = ...).
  exportName? : string;
  /// The node being exported.
  exportedNode : ts.Node;
};

export function getDeclarationKind(decl : ts.Node) : string {
  switch (decl.kind) {
    case ts.SyntaxKind.TypeAliasDeclaration:
        return "jstypealias";
    case ts.SyntaxKind.Constructor:
        return "jsconstructor";
    case ts.SyntaxKind.TypeParameter:
        return "jstypeparameter";
    case ts.SyntaxKind.Parameter:
        return "jsparameter";
    case ts.SyntaxKind.VariableDeclaration:
        return "jsvariable";
    case ts.SyntaxKind.PropertySignature:
    case ts.SyntaxKind.PropertyDeclaration:
    case ts.SyntaxKind.PropertyAssignment:
        return "jsproperty";
    case ts.SyntaxKind.EnumMember:
        return "jsenummember"
    case ts.SyntaxKind.MethodSignature:
    case ts.SyntaxKind.MethodDeclaration:
        return "jsmethod";
    case ts.SyntaxKind.FunctionDeclaration:
        return "jsfunction";
    case ts.SyntaxKind.GetAccessor:
        return "jsgetaccessor";
    case ts.SyntaxKind.SetAccessor:
        return "jssetaccessor";
    case ts.SyntaxKind.ClassDeclaration:
        return "jsclass";
    case ts.SyntaxKind.InterfaceDeclaration:
        return "jsinterface";
    case ts.SyntaxKind.EnumDeclaration:
        return "jsenum";
    case ts.SyntaxKind.ModuleDeclaration:
        return "jsmodule";
    case ts.SyntaxKind.ImportDeclaration:
        return "jsimport";
    case ts.SyntaxKind.Identifier:
        let ident = <ts.Identifier>decl;
        if (ident.text == 'exports') {
          return "jsexport";
        }
        return "jsident" + ident.text;
    default:
        return "jsunknown" + decl.kind;
  }
}

export function getCanonicalNode(node : ts.Node) {
  return tsu.isDeclaration(node)
      ? (<ts.Declaration>node).name
      : node.parent && tsu.isDeclaration(node.parent)
      ? (<ts.Declaration>node.parent).name
      : node;
}

export function extractExportFromToken(lang : ts.LanguageService,
                                       job : jobs.Job, token : ts.Node)
                                      : ExportInfo[] {
  if (!tokenIsExports(lang, job, token)) {
    return [];
  }
  var info : ExportInfo = {exportedNode: token};
  if (token.parent && token.parent.parent &&
      token.parent.kind == ts.SyntaxKind.PropertyAccessExpression &&
      token.parent.parent.kind == ts.SyntaxKind.BinaryExpression) {
    // exports.foo = bar;
    // module.exports = baz;
    let assignment = <ts.BinaryExpression>token.parent.parent;
    let propAccess = <ts.PropertyAccessExpression>token.parent;
    info.exportName = propAccess.name.text;
    info.exportedNode = assignment.right;
  } else if (token.parent &&
             token.parent.kind == ts.SyntaxKind.BinaryExpression) {
    // exports = bar;
    let assignment = <ts.BinaryExpression>token.parent;
    info.exportedNode = assignment.right;
  } else {
    return [];
  }
  return [info];
}
