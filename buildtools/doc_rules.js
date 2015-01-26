'use strict';

var path = require('path');

var entity = require('./entity.js');
var rule = require('./rule.js');

/*
asciidoc rule:
  Generate documentation from AsciiDoc sources

  Inputs
    srcs: the asciidoc source files
  Properties:
    asciidoc_backend: backend value passed to asciidoc
    asciidoc_attrs: map of attributes passed to asciidoc

Example:
{
  "name": "some_doc",
  "kind": "asciidoc",
  "properties": {
    "asciidoc_backend": "xhtml11",
    "asciidoc_attrs": {
      "toc2": true
    }
  },
  "inputs": {
    "srcs": [
      "doc.txt"
    ]
  }
}
*/
function AsciiDoc(engine) {
  this.engine = engine;
}
AsciiDoc.prototype = Object.create(rule.Rule.prototype);
AsciiDoc.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.allFilesFilter);
  var backend = target.getPropertyValue('asciidoc_backend') || 'html';
  var out = target.getFileNode(
      target.getRoot('doc') + extForBackend(target, backend));
  var args = [];
  var attrs = target.getPropertyValue('asciidoc_attrs') || {};
  for (var attr in attrs) {
    if (attrs[attr]) {
      args.push('--attribute=' + attr + '=' + attrs[attr]);
    } else {
      args.push('--attribute=' + attr + '!');
    }
  }
  var confs = rule.getAllOutputsFor(target.inputsByKind['confs'], 'build',
                                    rule.allFilesFilter);
  for (var i = 0; i < confs.length; i++) {
    args.push('--conf-file=' + confs[i].getPath());
  }
  var deps = confs
      .concat(rule.getAllOutputsFor(target.inputsByKind['tools'], 'build'))
      .concat(rule.getAllOutputsFor(target.inputsByKind['data'], 'build',
                                    rule.allFilesFilter));
  if (target.getPropertyValue('asciidoc_partial')) {
    args.push('--no-header-footer');
  }
  return {
    BUILD: [{
      rule: 'asciidoc',
      inputs: srcs,
      outs: [out],
      implicits: deps,
      vars: {
        backend: backend,
        args: args.join(' ')
      }
    }]
  };
};

function extForBackend(target, backend) {
  if (backend.indexOf('html') > -1) {
    return '.html';
  }
  console.error('ERROR: unhandled backend "' + backend + '"');
  process.exit(1);
}

exports.register = function(engine) {
  engine.addRule('asciidoc', new AsciiDoc(engine));
};
