'use strict';

var child_process = require('child_process');
var fs = require('fs');
var util = require('util');

var engine = require('./engine.js');
var rule = require('./rule.js');
var shared = require('./shared.js');

/**
 * Returns the unique subset of normalized labels in the given array.
 */
function dedupLabels(labels) {
  var uniqueLabels = {};
  for (var k = 0; k < labels.length; k++) {
    var label = labels[k];
    var colon = label.indexOf(':');
    var lastSlash = label.lastIndexOf('/');
    if (lastSlash >= 0 && colon > lastSlash) {
      var name = label.substring(colon + 1);
      var pkg = label.substring(lastSlash + 1, colon);
      if (pkg == name) {
        label = label.substring(0, colon);
      }
    }
    uniqueLabels[label] = true;
  }
  return Object.keys(uniqueLabels);
}

/**
 * Returns a well-formatted version of the given CAMPFIRE file contents.
 */
function camper(input) {
  try {
    // Try to parse YAML 1.2 for convenience when writing CAMPFIRE files
    var parsed = require('js-yaml').safeLoad(input);
  } catch (e) {
    var parsed = JSON.parse(input);
  }
  for (var j = 0; j < parsed.length; j++) {
    if (parsed[j].kind === 'cc_external_lib') {
      // Order matters for these rules.
      continue;
    }
    for (var inputKind in parsed[j].inputs) {
      var labels = dedupLabels(parsed[j].inputs[inputKind]);
      labels.sort(function(a, b) {
        if (a.indexOf(':') === 0) {
          if (b.indexOf(':') === 0) {
            return a.localeCompare(b);
          } else {
            return b.indexOf('/') === 0 ? -1 : 1;
          }
        } else if (b.indexOf(':') === 0) {
          return a.indexOf('/') === 0 ? 1 : -1;
        }
        return a.localeCompare(b);
      });
      parsed[j].inputs[inputKind] = labels;
    }
  }
  // TODO(schroederc): store as YAML?
  return JSON.stringify(parsed, undefined, 2) + '\n';
}

/**
 * Representation of a top-level campfire command (e.g. build/test/clean).
 */
function Command(run, help) {
  this.run = run;
  this.help = help;
}

/**
 * Top-level campfire commands.
 */
exports.commands = {
  build: new Command(function(engine, args) {
    engine.ninjaCommand(rule.kinds.BUILD, args, true);
  }, 'Builds provided targets'),
  run: new Command(function(engine, args) {
    var target = args.shift();
    engine.runTarget(target, args);
  }, 'Builds and runs an executable target with the following arguments'),
  test: new Command(function(engine, args) {
    engine.ninjaCommand(rule.kinds.TEST, args, true);
  }, 'Builds provided targets and runs the set of targets that are tests'),
  bench: new Command(function(engine, args) {
    engine.ninjaCommand(rule.kinds.BENCH, args, true);
  }, 'Builds provided targets and runs the set of targets that are benchmarks'),
  camper: new Command(function(engine, args) {
    if (args[0] === '-c') {
      // Check if a file is formatted correctly
      var input = fs.readFileSync(args[1]);
      var printed = camper(input);
      if (input != printed) {
        console.error('CAMPFIRE file is not formatted: ' + args[1]);
        process.exit(1);
      }
    } else {
      // Format each CAMPFIRE file in the repository
      var files = engine.findBuildFiles();
      for (var i = 0; i < files.length; i++) {
        try {
          var input = fs.readFileSync(files[i]);
          var printed = camper(input);
          if (input != printed) {
            if (args.length === 0 || args[0] != '-n') {
              console.log('Rewriting ' + files[i]);
              fs.writeFileSync(files[i], printed);
            } else {
              console.log('Would rewrite ' + files[i]);
            }
          }
        } catch (e) {
          console.error("Camper error on file '" + files[i] + "': " + e);
          continue;
        }
      }
      console.log(process.env['USER'] + ' is one happy camper');
    }
  }, 'Formats CAMPFIRE files, use -n to see files to be rewritten' +
      ' without applying the rewrite.'),
  clean: new Command(function(engine, args) {
    if (fs.existsSync('campfire-out')) {
      shared.rmdirs('campfire-out');
    }
    if (fs.existsSync('build.ninja')) {
      fs.unlinkSync('build.ninja');
    }
  }, 'Cleans campfire state and output'),
  'package': new Command(function(engine, args) {
    engine.ninjaCommand(rule.kinds.PACKAGE, args, true);
  }, 'Creates docker packages for provided targets'),
  extract: new Command(function(engine, args) {
    engine.ninjaCommand(rule.kinds.EXTRACT, args, true);
  }, 'Extract kindex compilations'),
  query: new Command(function(engine, args) {
    engine.query(args[0]);
  }, 'Queries build rule information from CAMPFIRE files'),
  ninja: new Command(function(engine, args) {
    var kindProperty = engine.settings.properties['ninja_kind'] || 'BUILD';
    var kind = rule.kinds[kindProperty.toUpperCase()];
    if (!kind) {
      console.error('Unknown build kind ' + kindProperty);
      console.error('Known kinds: ' + Object.keys(rule.kinds));
      process.exit(1);
    }
    if (engine.settings.properties['run_ninja']) {
      console.warn('WARNING: `campfire ninja --run_ninja` is deprecated');
      console.warn('  Use `campfire ' + kind.name.toLowerCase() + '` instead');
    }
    engine.ninjaCommand(kind, args, engine.settings.properties['run_ninja']);
  }, 'Converts CAMPFIRE files to an equivalent build.ninja file'),
  show_config: new Command(function(engine, args) {
    console.log(
        util.inspect(engine.settings.properties, {depth: undefined }));
  }, 'Prints the configuration that will be used for the build.')
};
