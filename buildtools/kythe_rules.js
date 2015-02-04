'use strict';

var path = require('path');

var entity = require('./entity.js');
var rule = require('./rule.js');

/*
verifier_test rule:
  Given a compilation target, extract a .kindex file, analyze it, and verify the
  outputs.

  Binary targets (e.g. //kythe/cxx/verifier) are baked into the rule. The
  extractor and indexer used for the use is determined by the "language"
  property.  To add a new language, one must add to getNinjaBuilds.

  Inputs
    srcs: the source files with verifier assertions
    compilation: a single library whose compilation will be extracted and
                 analyzed
    entries: other verifier_test targets whose resulting storage entries are
             needed to complete the test's verification (usually used for
             cross-compilation unification tests)

  Supported languages:
    - java
    - c++

  Note: see the build targets in
        //kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/CAMPFIRE
        for further examples

Example:
{
  "name": "some_test",
  "kind": "verifier_test",
  "properties": {
    "language": "java"
  },
  "inputs": {
    "srcs": [
      "Basic.java",
      "Dep.java"
    ],
    "compilation": [
      ":some_test_lib"
    ],
    "entries": [
      ":dependent_verifier_test"
    ]
  }
}
{
  "name": "some_test_lib",
  "kind": "java_library",
  "inputs": {
    "srcs": [
      "Basic.java",
      "ClassWithoutAssertions.java",
      "Dep.java"
    ],
    "jars": [
      ":dependent_verifier_test_lib"
    ]
  }
}
*/

var VNAMES_CONFIG_FILE = 'kythe/data/vnames.json';
var VERIFIER_TARGET = '//kythe/cxx/verifier';
var JAVA_EXTRACTOR_TARGET = '//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor';
var JAVA_ANALYZER_TARGET = '//kythe/java/com/google/devtools/kythe/analyzers/java:indexer';
var CXX_EXTRACTOR_TARGET = '//kythe/cxx/extractor:cxx_extractor';
var CXX_ANALYZER_TARGET = '//kythe/cxx/indexer/cxx:indexer';

function VerifierTest(engine) {
  this.engine = engine;
}
VerifierTest.prototype = Object.create(rule.Rule.prototype);
VerifierTest.prototype.getNinjaBuilds = function(target) {
  var engine = target.rule.engine;
  var builds = [];

  var language = target.getProperty('language').value;

  var comps = target.inputsByKind['compilation'];
  if (comps.length != 1) {
    console.error('Must have exactly 1 compilation input for target ' + target.id);
    process.exit(1);
  }
  var compilation = comps[0];
  var compBuilds = compilation.rule.getBuilds(compilation, rule.kinds.BUILD);
  var compBuild;

  var extractor, analyzer;
  var extractorArguments = [];
  var analyzerArguments = [];

  // Language-specific analyzer/extractor binaries and arguments.
  if (language === 'java') {
    // Java
    extractor = rule.getExecutable(engine, JAVA_EXTRACTOR_TARGET);
    analyzer = rule.getExecutable(engine, JAVA_ANALYZER_TARGET);

    for (var i = 0; i < compBuilds.length; i++) {
      if (compBuilds[i].rule === 'javac') {
        compBuild = compBuilds[i];
      }
    }

    extractorArguments.append(engine.settings.properties['javac_opts'] || []);
    extractorArguments.append([
      '-cp',
      "'" + compilation.rule.getClasspath(compilation) + "'"
    ]);
    extractorArguments.append(rule.getPaths(
        rule.getAllOutputsFor(compilation.inputsByKind['srcs'], 'build',
                              rule.fileFilter('src_file', '.java'))));
  } else if (language === 'c++') {
    // C++
    extractor = rule.getExecutable(engine, CXX_EXTRACTOR_TARGET);
    analyzer = rule.getExecutable(engine, CXX_ANALYZER_TARGET);

    for (var i = 0; i < compBuilds.length; i++) {
      if (compBuilds[i].rule === 'cpp_compile') {
        compBuild = compBuilds[i];
      }
    }

    extractorArguments.append(rule.getPaths(compBuild.inputs));
    extractorArguments.push(compBuild.vars.copts);

    analyzerArguments.push('--ignore_unimplemented=true');
  } else {
    console.error('No verifier_test implementation for language: ' + language);
    process.exit(1);
  }

  if (!extractor) {
    console.error('ERROR: verifier_test extractor missing for ' + language);
    process.exit(1);
  } else if (!analyzer) {
    console.error('ERROR: verifier_test analyzer missing for ' + language);
    process.exit(1);
  } else if (!compBuild) {
    console.error('ERROR: verifier_test compilation for ' + target.id +
        ' (' + compilation.id + ') missing compilation build');
    process.exit(1);
  }

  target.addParent(analyzer.owner);
  target.addParent(extractor.owner);

  var kindex = target.getFileNode(target.getRoot('test') + '/compilation.kindex',
                               'kindex');
  builds.push(ninjaExtractor(extractor, compBuild, extractorArguments, kindex));

  var entriesFile =
      target.getFileNode(target.getRoot('test') + '/entries', 'entries');
  builds.push(ninjaAnalyzer(target,
                            analyzer, analyzerArguments, kindex, entriesFile));

  var entries = [entriesFile].concat(rule.getAllOutputsFor(
      target.inputsByKind['entries'], 'test',
      rule.fileFilter('entries')));
  builds.push(ninjaVerifier(target, entries));

  return {TEST: builds};
};

exports.javaNinjaExtractor = function(compilationTarget, compilationBuild, kindex) {
  var engine = compilationTarget.rule.engine;
  var extractor = rule.getExecutable(engine, JAVA_EXTRACTOR_TARGET);
  var extractorArguments = engine.settings.properties['javac_opts'] || [];
  extractorArguments = extractorArguments.concat([
    '-cp',
    "'" + compilationTarget.rule.getClasspath(compilationTarget) + "'"
  ]);
  extractorArguments.append(rule.getPaths(compilationBuild.inputs));
  return ninjaExtractor(extractor, compilationBuild, extractorArguments, kindex);
};

exports.cxxNinjaExtractor = function(compilationTarget, compilationBuild, kindex) {
  var engine = compilationTarget.rule.engine;
  var extractor = rule.getExecutable(engine, CXX_EXTRACTOR_TARGET);
  var args = rule.getPaths(compilationBuild.inputs).concat(compilationBuild.vars.copts);
  return ninjaExtractor(extractor, compilationBuild, args, kindex);
};

function ninjaExtractor(extractor, compBuild, extractorArguments, kindex) {
  var deps = compBuild.inputs
      .concat(compBuild.implicits || [])
      .concat(compBuild.ordered || []);
  var vnamesConfig = new entity.File(VNAMES_CONFIG_FILE);
  deps.push(vnamesConfig);
  return {
    rule: 'kythe_extractor',
    inputs: [extractor],
    implicits: deps,
    ordered: compBuild.ordered,
    outs: [kindex],
    vars: {
      args: extractorArguments.join(' '),
      vnames: vnamesConfig.getPath()
    }
  };
}

function ninjaAnalyzer(target,
                       analyzer, analyzerArguments, kindex, entriesFile) {
  return {
    rule: 'kythe_analyzer',
    inputs: [kindex],
    implicits: [analyzer],
    outs: [entriesFile],
    vars: {
      analyzer: analyzer.getPath(),
      args: analyzerArguments.join(' ')
    }
  };
}

function ninjaVerifier(target, entries) {
  var verifier = rule.getExecutable(target.rule.engine, VERIFIER_TARGET);
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build');
  var logFile =
      target.getFileNode(target.getRoot('test') + '/testlog', 'test_log');
  return {
    rule: 'kythe_verifier',
    inputs: srcs,
    implicits: [verifier].concat(entries),
    outs: [target.getFileNode(target.getRoot('test') + '.done', 'done_marker')],
    vars: {
      entries: rule.getPaths(entries).join(' '),
      log: logFile.getPath(),
      verifier: verifier.getPath()
    }
  };
}

exports.register = function(engine) {
  engine.addRule('verifier_test', new VerifierTest(engine));
};
