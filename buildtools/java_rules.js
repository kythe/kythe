'use strict';

var fs = require('fs');
var path = require('path');

var entity = require('./entity.js');
var kythe_rules = require('./kythe_rules.js');
var rule = require('./rule.js');

function JavaLibrary(engine) {
  this.engine = engine;
}

JavaLibrary.prototype = new rule.Rule();
JavaLibrary.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.fileFilter('src_file', '.java'));
  var deps = rule.getAllOutputsFor(target.inputsByKind['jars'], 'build',
                                   rule.fileFilter('java_jar'));
  var javacBuild = {
    rule: 'javac',
    inputs: srcs,
    implicits: deps,
    outs: [target.getFileNode(target.getRoot('bin') + '.jar', 'java_jar')],
    vars: {
      classpath: this.getClasspath(target)
    }
  };
  var kindex =
      target.getFileNode(target.getRoot('gen') + '.java.kindex', 'kindex');
  return {
    BUILD: [javacBuild],
    EXTRACT: [
      kythe_rules.javaNinjaExtractor(target, javacBuild, kindex)]
  };
};
JavaLibrary.prototype.getClasspath = function(target) {
  return rule.getPaths(
      rule.getAllOutputsFor(target.inputsByKind['jars'], 'build',
                            rule.fileFilter('java_jar'))).join(':');
};

function JavaBinary(engine) {
  this.engine = engine;
}

JavaBinary.prototype = new JavaLibrary();
JavaBinary.prototype.getExecutable = function(target) {
  return target.getFileNode(target.getRoot('bin'), 'java_shell');
};
JavaBinary.prototype.getNinjaBuilds = function(target) {
  var builds = JavaLibrary.prototype.getNinjaBuilds.call(this, target);
  var jarDeps =
      rule.getAllOutputsRecursiveFor(target.inputsByKind['jars'], 'build',
                                     rule.fileFilter('java_jar'));
  jarDeps.append(builds.BUILD.mapcat(function(b) { return b.outs; }));
  builds.BUILD.push({
    rule: 'java_shell',
    inputs: [],
    implicits: jarDeps,
    outs: [this.getExecutable(target)],
    vars: {
      main: target.properties['main_class'].value,
      classpath: rule
          .getPaths(jarDeps, target.rule.engine.campfireRoot)
          .join(':')
    }
  });
  return builds;
};

function JavaTest(engine) {
  this.engine = engine;
}

JavaTest.prototype = new JavaLibrary();
JavaTest.prototype.getNinjaBuilds = function(target) {
  var builds = JavaLibrary.prototype.getNinjaBuilds.call(this, target);
  var jarDeps =
      rule.getAllOutputsRecursiveFor(target.inputsByKind['jars'], 'build',
                                     rule.fileFilter('java_jar'));
  jarDeps.append(builds.BUILD.mapcat(function(b) { return b.outs; }));
  var testBinary = target.getFileNode(target.getRoot('test'), 'java_shell');
  builds.BUILD.push({
    rule: 'java_shell',
    inputs: [],
    outs: [testBinary],
    implicits: jarDeps,
    vars: {
      main: 'org.junit.runner.JUnitCore',
      args: target.properties['test_class'].value,
      classpath: rule.getPaths(jarDeps).join(':')
    }
  });
  var logFile = target.getFileNode(target.getRoot('test') + '.log', 'test_log');
  builds.TEST = [{
    rule: 'run_test',
    inputs: [testBinary],
    outs: [target.getFileNode(target.getRoot('test') + '.done', 'done_marker')],
    vars: {
      log: logFile.getPath()
    }
  }];
  return builds;
};

function JavaDeployJar(engine) {
  this.engine = engine;
}

JavaDeployJar.prototype = new rule.Rule();
JavaDeployJar.prototype.getNinjaBuilds = function(target) {
  var jars =
      rule.getAllOutputsRecursiveFor(target.inputsByKind['jars'], 'build',
                                     rule.fileFilter('java_jar'));
  return [{
    rule: 'java_deploy_jar',
    inputs: jars,
    outs: [target.getFileNode(target.getRoot('bin') + '.jar',
                              'java_deploy_jar')],
    vars: {
      main: target.properties['main_class'].value
    }
  }];
};

function JavaExternalJar(engine) {
  this.engine = engine;
}

JavaExternalJar.prototype = new rule.Rule();
JavaExternalJar.prototype.getOutputsFor = function(target, kind) {
  if (target.outs) {
    return target.outs;
  }
  var inputs = rule.getAllOutputsFor(target.inputsByKind['srcs'],
                                     kind, rule.fileFilter('src_file', '.jar'));

  target.outs = [];
  for (var i = 0; i < inputs.length; i++) {
    inputs[i].kind = 'java_jar';
    target.outs.push(inputs[i]);
  }
  return target.outs;
};

exports.register = function(engine) {
  engine.addRule('java_library', new JavaLibrary(engine));
  engine.addRule('java_binary', new JavaBinary(engine));
  engine.addRule('java_test', new JavaTest(engine));
  engine.addRule('java_deploy_jar', new JavaDeployJar(engine));
  engine.addRule('java_external_jar', new JavaExternalJar(engine));
};
