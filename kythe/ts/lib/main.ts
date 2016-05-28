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

import fs = require('fs');
import ts = require('typescript');
import tsu = require('./tsutilities');
import kythe = require('./kythe');
import jobs = require('./job');
import au = require('./astutilities');
import su = require('./sbutilities');
import enc = require('./encoding');

function index(job : jobs.Job) {
  // Dump the job without the classifier object.
  var saved_classifier = job.classifier;
  job.classifier = new kythe.PathClassifier([]);
  console.warn('indexing', job);
  job.classifier = saved_classifier;

  const sourceFileVersion = "1";

  let snapshots : ts.Map<ts.IScriptSnapshot> = {};
  let program = createFilesystemProgram();
  let filenames = program.getSourceFiles().map(file => file.fileName);
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
    var isRequestedFile = false;
    for (var i = 0; i < job.files.length; ++i) {
      if (job.files[i] == file.fileName) {
        isRequestedFile = true;
        break;
      }
    }
    if (!isRequestedFile) {
      console.warn('implicit dependency on', file.fileName);
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
    // TODO(zarko): this is not a very polite thing to do.
    let imports = (<any>file).imports;
    let resolvedModules = (<any>file).resolvedModules;
    if (imports && resolvedModules) {
      for (var n = 0; n < imports.length; ++n) {
        let importPos = imports[n].pos;
        let importEnd = imports[n].end;
        let importText = imports[n].text;
        if (importText && resolvedModules.hasOwnProperty(importText)) {
          let resolvedModule = resolvedModules[importText];
          if (resolvedModule) {
            let resolvedFileName = resolvedModule.resolvedFileName;
            if (resolvedFileName && importPos && importEnd) {
              // These should appear as references like everything else below,
              // but they're not showing up?
              let refAnchorVName = kythe.anchor(vname,
                  enc.getUtf8PositionOfSourcePosition(file, importPos),
                  enc.getUtf8PositionOfSourcePosition(file, importEnd));
              let targetVName = job.classifier.classify(resolvedFileName);
              kythe.write(kythe.edge(refAnchorVName, "ref/includes",
                                     targetVName));
            }
          }
        }
      }
    }
    classify(vname, file, syntax);
    classify(vname, file, sema);
  });

  checkForErrors(program);

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
          var realpath = fs.realpathSync(filename);
          var isImplicit = false;
          for (var i = 0; i < job.implicits.length; ++i) {
            if (job.implicits[i] == realpath) {
              isImplicit = true;
              break;
            }
          }
          if (!isImplicit) {
            for (var i = 0; i < job.implicits.length; ++i) {
              sourceFile.referencedFiles.push({
                  fileName: job.implicits[i], pos: 0, end: 0
              });
            }
          }
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
      readFile: filename => {
        let fileBuf = ts.sys.readFile(filename);
        // If we don't do this, non-source files (e.g., package.json) won't
        // be available during later phases.
        snapshots[filename] = ts.ScriptSnapshot.fromString(fileBuf);
        return fileBuf;
      }
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

  // Used to find a VName for a definition referenced by an otherwise
  // unremarkable reference (like one that isn't part of a function call and
  // thus would need to point to something other than the canonical node for
  // tok).
  // defFile is the file in which tok is defined (or is the file
  // being referenced if tok is null).
  function translateSimpleReference(defFile : kythe.VName, tok : ts.Node) {
    if (tok && tok.kind !== ts.SyntaxKind.SourceFile) {
      return vnameForNode(defFile, tok);
    } else {
      return defFile;
    }
  }

  function indexReferencedFile(vname : kythe.VName, file : ts.SourceFile,
                               ref : ts.FileReference) {
    var refAnchorVName : kythe.VName = null;
    let defs = visitReferencesAt(file, ref.pos, (defFile, tok) => {
      refAnchorVName = refAnchorVName || textRangeAnchor(file, vname, ref);
      kythe.write(kythe.edge(refAnchorVName, "ref/includes",
          translateSimpleReference(defFile, tok)));
    });
  }

  // Find the nearest parent above node that would be a valid Kythe parent.
  // One may not exist (e.g. if this is a top-level definition).
  function findKytheParent(node : ts.Node) {
    while (node.parent) {
      switch (node.parent.kind) {
        case ts.SyntaxKind.FunctionDeclaration:
          return node.parent;
      }
      node = node.parent;
    }
    return undefined;
  }

  function findParentVName(fileVName : kythe.VName, node : ts.Node) {
    let parent = findKytheParent(node);
    return parent ? vnameForNode(fileVName, parent) : undefined;
  }

  function emitCommonDeclarationEntries(vbl : ts.Node, fullName : string,
      searchString : string, file : ts.SourceFile, fileVName : kythe.VName,
      spanStart : number, spanLength : number) : kythe.VName {
    let vname = vnameForNode(fileVName, vbl);
    let defSiteAnchor = kythe.anchor(fileVName,
        enc.getUtf8PositionOfSourcePosition(file, spanStart),
        enc.getUtf8PositionOfSourcePosition(file, spanStart + spanLength));
    kythe.write(kythe.edge(defSiteAnchor, "defines/binding", vname));
    kythe.write(kythe.edge(vname, "named",
        kythe.nameFromQualifiedName(fullName, "n")));
    if (searchString != fullName) {
      kythe.write(kythe.edge(vname, "named", kythe.name([searchString], "n")));
    }
    return vname;
  }

  function visitParameterDeclaration(
      vbl : ts.ParameterDeclaration, fullName : string, searchString : string,
      file : ts.SourceFile, fileVName : kythe.VName, spanStart : number,
      spanLength : number) {
    let vname = emitCommonDeclarationEntries(vbl, fullName, searchString,
        file, fileVName, spanStart, spanLength);
    kythe.write(kythe.fact(vname, "node/kind", "variable"));
    if (vbl.parent && vbl.parent.kind == ts.SyntaxKind.FunctionDeclaration) {
      let parentFn = <any>(vbl.parent);
      for (var i = 0; i < parentFn.parameters.length; ++i) {
        if (parentFn.parameters[i] == vbl) {
          kythe.write(kythe.paramEdge(vnameForNode(fileVName, vbl.parent),
              vname, i));
          break;
        }
      }
    }
    // TODO(zarko): completes edges and completeness facts.
  }

  function visitVariableDeclaration(
      vbl : ts.VariableDeclaration, fullName : string, searchString : string,
      file : ts.SourceFile, fileVName : kythe.VName, spanStart : number,
      spanLength : number) {
    let vname = emitCommonDeclarationEntries(vbl, fullName, searchString,
        file, fileVName, spanStart, spanLength);
    kythe.write(kythe.fact(vname, "node/kind", "variable"));
    let parentVName = findParentVName(fileVName, vbl);
    if (parentVName) {
      kythe.write(kythe.edge(vname, "childof", parentVName));
    }
    // TODO(zarko): completes edges and completeness facts.
  }

  function visitFunctionDeclaration(
      fn : ts.FunctionDeclaration, fullName : string, searchString : string,
      file : ts.SourceFile, fileVName : kythe.VName, spanStart : number,
      spanLength : number) {
    let vname = emitCommonDeclarationEntries(fn, fullName, searchString,
        file, fileVName, spanStart, spanLength);
    kythe.write(kythe.fact(vname, "node/kind", "function"));
    // TODO(zarko): completes edges.
    kythe.write(kythe.fact(vname, "complete",
        fn.body ? "definition" : "incomplete"));
  }

  function visitPropertyAssignment(
      prop : ts.PropertyAssignment, fullName : string, searchString : string,
      file : ts.SourceFile, fileVName : kythe.VName, spanStart : number,
      spanLength : number) {
    let vname = emitCommonDeclarationEntries(prop, fullName, searchString,
        file, fileVName, spanStart, spanLength);
    kythe.write(kythe.fact(vname, "node/kind", "variable"));
  }

  function visitDeclaration(declaration : ts.Declaration,
      file : ts.SourceFile, fileVName : kythe.VName, spanStart : number,
      spanLength : number) {
    let searchString = declaration.name
        && tsu.declarationNameToString(declaration.name);
    let definitionKind = au.getDeclarationKind(declaration);
    let fullName = su.getQualifiedName(declaration);

    switch (declaration.kind) {
      case ts.SyntaxKind.FunctionDeclaration:
        return visitFunctionDeclaration(<ts.FunctionDeclaration>declaration,
            fullName, searchString, file, fileVName, spanStart, spanLength);
      case ts.SyntaxKind.VariableDeclaration:
        return visitVariableDeclaration(<ts.VariableDeclaration>declaration,
            fullName, searchString, file, fileVName, spanStart, spanLength);
      case ts.SyntaxKind.Parameter:
        return visitParameterDeclaration(<ts.ParameterDeclaration>declaration,
            fullName, searchString, file, fileVName, spanStart, spanLength);
      case ts.SyntaxKind.PropertyAssignment:
        return visitPropertyAssignment(<ts.PropertyAssignment>declaration,
            fullName, searchString, file, fileVName, spanStart, spanLength);
    }

    let defSiteAnchor = kythe.anchor(fileVName,
        enc.getUtf8PositionOfSourcePosition(file, spanStart),
        enc.getUtf8PositionOfSourcePosition(file, spanStart + spanLength));
    let defVName = vnameForNode(fileVName, declaration);
    kythe.write(kythe.edge(defSiteAnchor, "defines/binding", defVName));
    kythe.write(kythe.fact(defVName, "node/kind", definitionKind));
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
        visitDeclaration(declaration, file, vname, spanStart, spanLength);
      } else if (token.kind == ts.SyntaxKind.Identifier
                 && token.parent
                 && token.parent.kind === ts.SyntaxKind.LabeledStatement) {
        // TODO(zarko): index label.
      } else {
        let exportInfo = au.extractExportFromToken(lang, job, token);
        if (exportInfo && exportInfo.length != 0) {
          for (let xi = 0, xn = exportInfo.length; xi < xn; ++xi) {
            let xport = exportInfo[xi];
            let defSiteAnchor = kythe.anchor(vname,
                enc.getUtf8PositionOfSourcePosition(file, spanStart),
                enc.getUtf8PositionOfSourcePosition(file,
                                                    spanStart + spanLength));
            let exportVName = vnameForNode(vname, token);
            kythe.write(kythe.edge(defSiteAnchor, "defines/binding",
                exportVName));
            kythe.write(kythe.fact(exportVName, "node/kind", "js/export"));
          }
        } else {
          var refSiteAnchor : kythe.VName = null;
          var refParent : ts.Node = null;
          var refParentVName : kythe.VName = null;
          visitReferencesAt(file, spanStart, (defFile, tok) => {
            refSiteAnchor = refSiteAnchor || kythe.anchor(vname,
                enc.getUtf8PositionOfSourcePosition(file, spanStart),
                enc.getUtf8PositionOfSourcePosition(file,
                                                    spanStart + spanLength));
            let canTok = au.getCanonicalNode(tok);
            if (canTok && canTok.parent) {
              if (canTok.parent.kind == ts.SyntaxKind.FunctionDeclaration
                  || canTok.parent.kind == ts.SyntaxKind.PropertyAssignment) {
                refParent = refParent
                    || tsu.getTokenAtPosition(file, spanStart);
                refParentVName = refParentVName
                    || findParentVName(vname, refParent);
                // x(f)
                if (refParent.parent && refParent.parent.kind
                    == ts.SyntaxKind.CallExpression) {
                  kythe.write(kythe.edge(refSiteAnchor, "ref/call",
                          vnameForNode(defFile, canTok.parent)));
                  if (refParentVName) {
                    kythe.write(kythe.edge(refSiteAnchor, "childof",
                        refParentVName));
                  }
                  return;
                }
                // [x.y](f)
                if (refParent.parent.parent && refParent.parent.parent.kind
                    == ts.SyntaxKind.CallExpression) {
                  let prop = vnameForNode(defFile, canTok.parent);
                  kythe.write(kythe.edge(refSiteAnchor, "ref/call", prop));
                  if (refParentVName) {
                    kythe.write(kythe.edge(refSiteAnchor, "childof",
                        refParentVName));
                  }
                  return;
                }
              }
            }
            kythe.write(kythe.edge(refSiteAnchor, "ref",
                translateSimpleReference(defFile, tok)));
          });
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
      // TODO(zarko): do not use source locations for semantic objects
      let defStart = target.getStart();
      copyName.signature = target.kind + "_" + defStart;
    } else {
      copyName.signature = "_Missing";
    }
    copyName.language = "ts";
    return copyName;
  }

  // For each definition D referenced from file at pos, calls fn with the VName
  // of D's containing file as well as the node for D itself.
  function visitReferencesAt(file : ts.SourceFile,
                             pos : number,
                             fn : (defFile:kythe.VName, tok:ts.Node) => void) {
    let defs = lang.getDefinitionAtPosition(file.fileName, pos);
    (defs ? defs : []).forEach((def : ts.DefinitionInfo) => {
      let defStart = def.textSpan.start;
      let defFile = program.getSourceFile(def.fileName);
      let token = tsu.getTouchingToken(defFile, defStart,
          /* includeItemAtEndPosition */ undefined);
      let defFileVName = fileVName(defFile);
      fn(defFileVName, token);
    });
  }

  function textRangeAnchor(file : ts.SourceFile, fileVName : kythe.VName,
                           range : ts.TextRange) {
    return kythe.anchor(fileVName,
        enc.getUtf8PositionOfSourcePosition(file, range.pos),
        enc.getUtf8PositionOfSourcePosition(file, range.end));
  }

  function textSpanAnchor(file : ts.SourceFile, fileVName : kythe.VName,
                          span : ts.TextSpan) {
    return kythe.anchor(fileVName,
        enc.getUtf8PositionOfSourcePosition(file, span.start),
        enc.getUtf8PositionOfSourcePosition(file, span.start + span.length));
  }

  function fileVName(file : ts.SourceFile) : kythe.VName {
    return job.classifier.classify(file.fileName);
  }
}

if (require.main === module) {
  var args = require('minimist')(process.argv.slice(2));
  // TODO(zarko): detect these externally.
  var options : ts.CompilerOptions = {
    target : ts.ScriptTarget.ES5,
    module: ts.ModuleKind.CommonJS,
    moduleResolution: ts.ModuleResolutionKind.NodeJs,
    allowNonTsExtensions: true,
    experimentalDecorators: true,
    rootDir: args.rootDir
  };
  var vnames : kythe.VNamesConfigFileEntry[] = [];
  if (process.env.KYTHE_VNAMES) {
    let vnamesText = fs.readFileSync(process.env.KYTHE_VNAMES,
        {encoding: 'utf8'});
    vnames = <kythe.VNamesConfigFileEntry[]>(JSON.parse(vnamesText));
    if (!vnames) {
      console.warn("can't understand ", process.env.KYTHE_VNAMES);
      process.exit(1);
    }
  }
  var classifier = new kythe.PathClassifier(vnames);
  var files : string[] = [];
  var implicits : string[] = [];
  for (var i = 0; i < args._.length; ++i) {
    if (args._[i].startsWith('implicit=')) {
      implicits.push(fs.realpathSync(args._[i].substr(9)));
    } else if (args._[i].length != 0) {
      files.push(fs.realpathSync(args._[i]));
    }
  }
  var job = {
    lib: ts.getDefaultLibFilePath(options),
    files: files,
    implicits: implicits,
    options: options,
    dontIndexDefaultLib: !!args.skipDefaultLib,
    classifier: classifier
  };
  index(job);
}
