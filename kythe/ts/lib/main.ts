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
import kythe = require('./kythe');
import jobs = require('./job');
import au = require('./astutilities');
import su = require('./sbutilities');

function index(job : jobs.Job) {
  console.warn('indexing', job);
  const sourceFileVersion = "1";

  let program = createFilesystemProgram();
  let filenames = program.getSourceFiles().map(file => file.fileName);
  let snapshots : ts.Map<ts.IScriptSnapshot> = {};
  let typechecker = program.getTypeChecker();

  program.getSourceFiles().forEach(file =>
      snapshots[file.fileName] = ts.ScriptSnapshot.fromString(file.text));

  const hostNeverCancelledToken : ts.HostCancellationToken = {
    isCancellationRequested: () => false
  };

  let langHost : ts.LanguageServiceHost = {
    getCancellationToken: () => hostNeverCancelledToken,
    getCompilationSettings: () => job.options,
    getDefaultLibFileName: _ => job.lib,
    getCurrentDirectory: () => ".",
    getLocalizedDiagnosticMessages: () => undefined,
    getScriptFileNames: () => filenames,
    getScriptVersion: _ => sourceFileVersion,
    getScriptSnapshot: name => snapshots[name],
    log: s => console.warn(s),
    trace: s => console.warn(s),
    error: s => console.error(s)
  };

  let documentRegistry : ts.DocumentRegistry = {
    acquireDocument: (filename, settings, snapshot, version) =>
        program.getSourceFile(filename),
    releaseDocument: (filename, settings) => undefined,
    updateDocument: (filename, settings, snapshot, version) =>
        program.getSourceFile(filename),
    reportStats: () => ""
  };

  let lang = ts.createLanguageService(langHost, documentRegistry);
  let defaultLibFileName = langHost.getDefaultLibFileName(job.options);

  program.getSourceFiles().forEach(file => {
    if (job.dontIndexDefaultLib && file.fileName == defaultLibFileName) {
      return;
    }
    console.warn('processing', file.fileName);
    let vname = fileVName(file);
    kythe.write(kythe.fact(vname, "node/kind", "file"));
    kythe.write(kythe.fact(vname, "text", file.text));
    let span = ts.createTextSpan(0, file.text.length);
    let syntax = lang.getEncodedSyntacticClassifications(file.fileName, span);
    let sema = lang.getEncodedSemanticClassifications(file.fileName, span);
    file.referencedFiles.forEach(ref => indexReferencedFile(vname, file, ref));
    classify(vname, file, syntax);
    classify(vname, file, sema);
  });

  checkForErrors(program);

  return;

  const neverCancelledToken : ts.CancellationToken = {
    isCancellationRequested: () => false,
    throwIfCancellationRequested: () => {}
  };

  // TODO(zarko): capture getSourceFile calls to capture a hermetic build
  // TODO(zarko): add implicit references based on project type
  // (e.g., sourceFile.referencedFiles.push(
  //    {fileName: "../../typings/node/node.d.ts", pos: 0, end: 0});)
  /// Creates a ts.Program that will read directly from the local file system.
  function createFilesystemProgram() : ts.Program {
    let host : ts.CompilerHost = {
      getSourceFile: (filename, languageVersion) => {
        if (ts.sys.fileExists(filename)) {
          let text = ts.sys.readFile(filename);
          let sourceFile = ts.createSourceFile(filename, text, languageVersion,
              /* setParentNodes=*/true);
          return sourceFile;
        }
      },
      getCancellationToken: () => neverCancelledToken,
      getCanonicalFileName: (filename) =>
          ts.sys.useCaseSensitiveFileNames ? filename : filename.toLowerCase(),
      useCaseSensitiveFileNames: () => ts.sys.useCaseSensitiveFileNames,
      getNewLine: () => "\n",
      getDefaultLibFileName: () => job.lib,
      writeFile: (filename, data, writeBOM) => { throw new Error("write?"); },
      getCurrentDirectory: () => ts.sys.getCurrentDirectory(),
      fileExists: filename => ts.sys.fileExists(filename),
      readFile: filename => ts.sys.readFile(filename)
    };
    return ts.createProgram(job.files, job.options, host);
  }

  function checkForErrors(program : ts.Program) {
    let diagnostics = program.getGlobalDiagnostics().concat(
        program.getSyntacticDiagnostics()).concat(
        program.getSemanticDiagnostics());

    var diagnosticErrs = diagnostics.map((d) => {
      var msg = '';
      if (d.file) {
        let pos = d.file.getLineAndCharacterOfPosition(d.start);
        let fn = d.file.fileName;
        msg += ` ${fn}:${pos.line + 1}:${pos.character + 1}`;
      }
      msg += ': ';
      msg += ts.flattenDiagnosticMessageText(d.messageText, '\n');
      return msg;
    });

    if (diagnosticErrs.length) {
      console.warn(diagnosticErrs.join('\n'));
    }
  }

  function indexReferencedFile(vname : kythe.VName, file : ts.SourceFile,
                               ref : ts.FileReference) {
    let defs = visitReferencesAt(vname, file, ref.pos);
    if (defs && defs.length !== 0) {
      let refAnchorVName = textRangeAnchor(vname, ref);
      defs.forEach(def => {
        kythe.write(kythe.edge(refAnchorVName, "ref/includes", def));
      });
    }
  }

  function classify(vname : kythe.VName, file : ts.SourceFile,
                    classifications : ts.Classifications) {
    let spans = classifications.spans;
    for (let i = 0, n = spans.length; i < n; i += 3) {
      let spanStart = spans[i], spanLength = spans[i + 1], clazz = spans[i + 2];
      if (clazz == ts.ClassificationType.whiteSpace
          || clazz == ts.ClassificationType.operator
          || clazz == ts.ClassificationType.punctuation
          || clazz == ts.ClassificationType.comment) {
        continue;
      }
      let token = tsu.getTokenAtPosition(file, spanStart);
      if (!token) {
        continue;
      }
      if (clazz == ts.ClassificationType.keyword &&
          token.kind != ts.SyntaxKind.ConstructorKeyword) {
        continue;
      }
      let declaration = su.getDeclarationForName(token);
      let vnames : kythe.VName[] = [];
      if (declaration) {
        // TODO(zarko): Emit and connect name nodes for searchString and
        // fullName.
        let searchString = declaration.name
            && tsu.declarationNameToString(declaration.name);
        let definitionKind = au.getDeclarationKind(declaration);
        let fullName = su.getQualifiedName(declaration);
        let defSiteAnchor = kythe.anchor(vname, spanStart,
            spanStart + spanLength);
        let defVName = vnameForNode(vname, declaration);
        kythe.write(kythe.edge(defSiteAnchor, "defines/binding", defVName));
        kythe.write(kythe.fact(defVName, "node/kind", definitionKind));
      } else if (token.kind == ts.SyntaxKind.Identifier
                 && token.parent
                 && token.parent.kind === ts.SyntaxKind.LabeledStatement) {
        // TODO(zarko): index label.
      } else {
        let exportInfo = au.extractExportFromToken(lang, job, token);
        if (exportInfo && exportInfo.length != 0) {
          for (let xi = 0, xn = exportInfo.length; xi < xn; ++xi) {
            let xport = exportInfo[xi];
            let defSiteAnchor = kythe.anchor(vname, spanStart,
                spanStart + spanLength);
            let exportVName = vnameForNode(vname, token);
            kythe.write(kythe.edge(defSiteAnchor, "defines/binding",
                exportVName));
            kythe.write(kythe.fact(exportVName, "node/kind", "js/export"));
          }
        } else {
          vnames = visitReferencesAt(vname, file, spanStart);
          let refSiteAnchor = kythe.anchor(vname, spanStart,
              spanStart + spanLength);
          for (let vi = 0, vn = vnames.length; vi < vn; ++vi) {
            kythe.write(kythe.edge(refSiteAnchor, "ref", vnames[vi]));
          }
        }
      }
    }
  }

  function vnameForNodeNoFile(node : ts.Node) : kythe.VName {
    return vnameForNode(fileVName(tsu.getSourceFileOfNode(node)), node);
  }

  function vnameForNode(defFileVName : kythe.VName, node : ts.Node)
      : kythe.VName {
    let target = au.getCanonicalNode(node);
    let copyName = kythe.copyVName(defFileVName);
    if (target) {
      let defStart = target.getStart();
      copyName.signature = au.getDeclarationKind(target) + defStart;
    } else {
      copyName.signature = "_Missing";
    }
    return copyName;
  }

  function visitReferencesAt(vname : kythe.VName, file : ts.SourceFile,
                             pos : number) : kythe.VName[] {
    let vnames : kythe.VName[] = [];
    let defs = lang.getDefinitionAtPosition(file.fileName, pos);
    (defs ? defs : []).forEach((def : ts.DefinitionInfo) => {
      let defStart = def.textSpan.start;
      let defFile = program.getSourceFile(def.fileName);
      let token = tsu.getTouchingToken(defFile, defStart,
          /* includeItemAtEndPosition */ undefined);
      let defFileVName = fileVName(defFile);
      if (token && token.kind !== ts.SyntaxKind.SourceFile) {
        vnames.push(vnameForNode(defFileVName, token));
      } else {
        vnames.push(defFileVName);
      }
    });
    return vnames;
  }

  function textRangeAnchor(fileVName : kythe.VName, range : ts.TextRange) {
    return kythe.anchor(fileVName, range.pos, range.end);
  }

  function textSpanAnchor(fileVName : kythe.VName, span : ts.TextSpan) {
    return kythe.anchor(fileVName, span.start, span.start + span.length);
  }

  function fileVName(file : ts.SourceFile) : kythe.VName {
    // TODO(zarko): set a proper corpus.
    return kythe.vname('', file.fileName, 'ts', '',
        file.moduleName ? file.moduleName : 'nocorpus');
  }
}

if (require.main === module) {
  var args = require('minimist')(process.argv.slice(2), {base: 'string'});
  // TODO(zarko): detect these externally.
  var options : ts.CompilerOptions = {
    target : ts.ScriptTarget.ES5,
    module: ts.ModuleKind.CommonJS,
    moduleResolution: ts.ModuleResolutionKind.Classic,
    allowNonTsExtensions: true,
    experimentalDecorators: true,
    rootDir: args.rootDir
  };
  var job = {
    lib: ts.getDefaultLibFilePath(options),
    files: args._,
    options: options,
    dontIndexDefaultLib: !!args.skipDefaultLib
  };
  index(job);
}
