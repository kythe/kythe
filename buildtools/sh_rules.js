// sh_rules allows shell scripts to be executed as tests.
//
// The rule takes exactly 1 'src' input which will determine the results of the
// test by its exit code.
//
// Scripts are executed in the campfire root directory.
//
// See //buildtools/test/CAMPFIRE for an example use of sh_test.

'use strict';

var entity = require('./entity.js');
var rule = require('./rule.js');

function ShTest(engine) {
  this.engine = engine;
}
ShTest.prototype = new rule.Rule;
ShTest.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.allFilesFilter);
  if (srcs.length != 1) {
    console.error('ERROR: invalid sh_test "' + target.id +
        '"; must have exactly 1 src');
    process.exit(1);
  }
  var deps = rule.getAllOutputsFor(target.inputsByKind['tools'], 'build');
  deps.append(rule.getAllOutputsFor(target.inputsByKind['data'], 'build',
                                    rule.allFilesFilter));
  var logFile = target.getFileNode(target.getRoot('test') + '.log', 'test_log');
  return {
    TEST: [{
      rule: 'run_test',
      inputs: srcs,
      implicits: deps,
      outs: [target.getFileNode(target.getRoot('test') + '.done',
                                'done_marker')],
      vars: {
        log: logFile.getPath()
      }
    }]
  };
};

exports.register = function(engine) {
  engine.addRule('sh_test', new ShTest(engine));
};
