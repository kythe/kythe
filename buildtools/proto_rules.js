'use strict';

var path = require('path');

var entity = require('./entity.js');
var go_rules = require('./go_rules.js');
var rule = require('./rule.js');

function getProtoImportMappings(owner, output, seen) {
  if (!output) {
    var results = [];
    getProtoImportMappings(owner, results, {});
    return results;
  }
  if (owner.inputsByKind['proto_libs']) {
    for (var i = 0; i < owner.inputsByKind['proto_libs'].length; i++) {
      var input = owner.inputsByKind['proto_libs'][i];
      if (!seen[input.id]) {
        seen[input.id] = true;
        if (input.rule.__proto__ == ProtoLibrary.prototype) {
          getProtoImportMappings(input, output, seen);
        }
      }
    }
  }
  var srcs = rule.getAllOutputsFor(owner.inputsByKind['srcs'], 'build',
                                   rule.fileFilter('src_file', '.proto'));
  output.push('M' + srcs[0].getPath() + '=' + owner.asPath());
}

function ProtoLibrary(engine) {
  this.engine = engine;
}

ProtoLibrary.prototype = new rule.Rule();
ProtoLibrary.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.fileFilter('src_file', '.proto'));

  var builds = [];
  if (target.properties.java_api && target.properties.java_api.value) {
    builds.push(javaNinjaBuild(target, srcs));
  }
  if (target.properties.go_api && target.properties.go_api.value) {
    builds.push(goNinjaBuild(target, srcs));
  }
  if (target.properties.cc_api && target.properties.cc_api.value) {
    builds.push(ccNinjaBuild(target, srcs));
  }

  return builds;
};

function javaNinjaBuild(target, srcs) {
  var jars = rule.getAllOutputsFor(target.inputsByKind['jars'],
                                   'build', rule.fileFilter('java_jar'));
  jars.append(rule.getAllOutputsFor(target.inputsByKind['proto_libs'],
                                    'build', rule.fileFilter('java_jar')));
  return {
    rule: 'protoc_java',
    phony: target.id + '_java',
    inputs: srcs,
    implicits: jars,
    outs: [target.getFileNode(target.getRoot('bin') + '.jar', 'java_jar')],
    vars: {
      classpath: rule.getPaths(jars).join(':')
    }
  };
}

function goNinjaBuild(target, srcs) {
  var outDir = path.join(target.getRoot('gen'), 'go');
  var includePaths = rule.getAllOutputsFor(target.inputsByKind['go_pkgs'], 'build',
                                           rule.propertyFilter('go_include_path'));
  includePaths.append(rule.getAllOutputsFor(target.inputsByKind['proto_libs'], 'build',
                                            rule.propertyFilter('go_include_path')));
  includePaths = includePaths.map(function(p) { return p.value; });
  includePaths.push(outDir);
  includePaths.push('campfire-out/go/pkg/linux_amd64/');
  var pkgs = rule.getAllOutputsFor(target.inputsByKind['go_pkgs'], 'build',
                                   rule.fileFilter('go_archive'));
  pkgs.append(rule.getAllOutputsFor(target.inputsByKind['proto_libs'],
                                    'build', rule.fileFilter('go_archive')));

  var protoImportMappings = getProtoImportMappings(target).join(',');
  if (protoImportMappings !== '') {
    protoImportMappings = ',' + protoImportMappings;
  }

  return {
    rule: 'protoc_go',
    phony: target.id + '_go',
    inputs: srcs,
    implicits: pkgs,
    outs: [target.getFileNode(go_rules.PACKAGE_DIR + target.asPath() + '.a',
                              'go_archive')],
    properties: [new entity.Property(target.id, 'go_include_path', outDir)],
    vars: {
      'package': target.asPath(),
      include: includePaths.join(' '),
      importpath: protoImportMappings,
      outdir: outDir
    }
  };
}

function ccNinjaBuild(target, srcs) {
  var includePaths = rule.getAllOutputsFor(target.inputsByKind['cc_libs'], 'build',
                                           rule.propertyFilter('cc_include_path'));
  includePaths.append(rule.getAllOutputsFor(target.inputsByKind['proto_libs'], 'build',
                                            rule.propertyFilter('cc_include_path')));
  var libs = rule.getAllOutputsFor(target.inputsByKind['proto_libs'], 'build',
                                   rule.fileFilter('cc_archive'));

  var archive = target.getFileNode(target.getRoot('bin') + '.a', 'cc_archive');
  var outputDir = path.join(target.getRoot('gen'), 'cxx');
  return {
    rule: 'protoc_cpp',
    phony: target.id + '_cpp',
    inputs: srcs,
    implicits: libs,
    outs: [archive],
    properties: [new entity.Property(target.id, 'cc_include_path', outputDir)],
    vars: {
      include: includePaths.map(function(p) { return p.value; }).join(' '),
      outdir: outputDir
    }
  };
}

exports.register = function(engine) {
  engine.addRule('proto_library', new ProtoLibrary(engine));
};
