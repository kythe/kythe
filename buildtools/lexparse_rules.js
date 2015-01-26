'use strict';

var path = require('path');

var entity = require('./entity.js');
var rule = require('./rule.js');

/*
genlex rule:
  Generate a flex lexer.

  Inputs
    srcs: the lexer definition (and, optionally, a genyacc target that will
    build a token table).

Example:
{
  "name": "lexer",
  "kind": "genlex",
  "inputs": {
    "srcs": [
      "lexer.lex",
      ":parser"
    ]
  }
}
*/
function GenLex(engine) {
  this.engine = engine;
}
GenLex.prototype = Object.create(rule.Rule.prototype);
GenLex.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.fileFilter('src_file', '.lex'));
  var deps = rule.getAllOutputsRecursiveFor(target.inputsByKind['srcs'], 'build',
                                            rule.fileFilter('gen_header_file'));
  var out = target.getFileNode(path.join(target.getRoot('gen'),
                                         'lexer.yy.cc'), 'src_file');
  return {
    BUILD: [{
      rule: 'genlex',
      inputs: srcs,
      implicits: deps,
      outs: [out]
    }]
  };
};

/*
genyacc rule:
  Generate a bison parser.

  Inputs
    srcs: the parser definition

Example:
{
  "name": "parser",
  "kind": "genyacc",
  "inputs": {
    "srcs": [
      "parser.yy"
    ]
  }
}
*/
function GenYacc(engine) {
  this.engine = engine;
}
GenYacc.prototype = Object.create(rule.Rule.prototype);
GenYacc.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.allFilesFilter);
  var mainOut = target.getFileNode(path.join(target.getRoot('gen'),
                                             'parser.yy.cc'),
                                   'src_file');
  var out = [
    mainOut,
    target.getFileNode(path.join(target.getRoot('gen'),
                                 'parser.yy.hh'), 'gen_header_file'),
    target.getFileNode(path.join(target.getRoot('gen'),
                                 'location.hh'), 'gen_header_file'),
    target.getFileNode(path.join(target.getRoot('gen'),
                                 'position.hh'), 'gen_header_file'),
    target.getFileNode(path.join(target.getRoot('gen'),
                                 'stack.hh'), 'gen_header_file')
  ];
  return {
    BUILD: [{
      rule: 'genyacc',
      inputs: srcs,
      vars: {
        main_out: mainOut.getPath()
      },
      outs: out,
      properties: [new entity.Property(target.id, 'cc_include_path',
                                       target.getRoot('gen'))]
    }]
  };
};

exports.register = function(engine) {
  engine.addRule('genlex', new GenLex(engine));
  engine.addRule('genyacc', new GenYacc(engine));
};
