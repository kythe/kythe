'use strict';

var fs = require('fs');
var path = require('path');

var commands = require('./commands.js');
var engine = require('./engine.js');
var shared = require('./shared.js');

var SETTINGS_LOCATIONS = ['.', process.env.HOME];
var SETTINGS_FILENAME = '.campfire_settings';

function findRoot(dir) {
  if (fs.existsSync(path.join(dir, SETTINGS_FILENAME))) {
    return dir;
  }
  if (dir == '/') {
    return undefined;
  }
  return findRoot(path.dirname(dir));
}
var cwd = global.cwd = process.cwd();
var campfireRoot = global.campfireRoot = findRoot(cwd);

if (!campfireRoot) {
  console.error('Unable to locate ' + SETTINGS_FILENAME + ' in any parent' +
      ' directory of current working directory');
  process.exit(1);
}
var relative = cwd.substring(campfireRoot.length + 1);

function inheritConfiguration(allConfigurations, inheritFrom,
                              configObject) {
  if (!(inheritFrom in allConfigurations)) {
    console.error("No such configuration '" + inheritFrom + "'");
    process.exit(1);
  }
  var parentConfig = allConfigurations[inheritFrom];
  if ('@inherit' in parentConfig) {
    inheritConfiguration(allConfigurations, parentConfig['@inherit'],
                         configObject);
  }
  for (var key in parentConfig) {
    if (key == '@inherit' || !parentConfig.hasOwnProperty(key)) {
      continue;
    } else if (key.charAt(0) == '+') {
      var actualKey = key.substr(1);
      if (actualKey in configObject) {
        configObject[actualKey] =
            configObject[actualKey].concat(parentConfig[key]);
      } else {
        configObject[actualKey] = parentConfig[key];
      }
    } else {
      configObject[key] = parentConfig[key];
    }
  }
}

function readSettings() {
  var settings = [];
  for (var i = 0; i < SETTINGS_LOCATIONS.length; i++) {
    var file = path.join(SETTINGS_LOCATIONS[i], SETTINGS_FILENAME);
    if (fs.existsSync(file)) {
      settings.push(JSON.parse(fs.readFileSync(file)));
    }
  }
  return shared.merge.apply(null, settings);
}

function runCampfire() {
  process.chdir(campfireRoot);
  var settings = readSettings();

  if (!settings.properties) {
    settings.properties = {};
  }
  if (!settings.configurations) {
    settings.configurations = {};
  }
  if (!settings.configurations.base) {
    settings.configurations.base = {};
  }
  var cmdlineConfig = {};
  var targets = [];
  var command = process.argv[2]; // ['node', 'main', <command>, args...]
  for (var i = 3; i < process.argv.length; i++) {
    var value = process.argv[i];
    // If an argument starts with '--', it is a property to be set in the build
    // configuration.  As a special case, for the 'run' command, we parse '--'
    // arguments only up to the first target spec (the executable target) and
    // leave every argument past that unchanged (for the consumption of the
    // target executable).
    if (value.indexOf('--') === 0 &&
        (command != 'run' || targets.length == 0)) {
      var rest = value.substring(2);
      var eqIndex = value.indexOf('=');
      if (eqIndex == -1) {
        cmdlineConfig[rest] = true;
      } else {
        var key = rest.substring(0, eqIndex - 2);
        var begin = rest.substring(eqIndex - 1);
        cmdlineConfig[key] = begin;
      }
      continue;
    }
    targets.push(value);
  }
  if (!('configuration' in cmdlineConfig)) {
    if ('configuration' in settings) {
      cmdlineConfig['configuration'] = settings['configuration'];
    } else {
      console.error('No default configuration and no configuration ' +
          'specified.');
      console.error('Specify a configuration with --configuration=foo ' +
          'or "configuration": "foo" in the top level of your ' +
          SETTINGS_FILENAME + ' file.');
      process.exit(1);
    }
  }
  cmdlineConfig['@inherit'] = cmdlineConfig['configuration'];
  settings.configurations['@cmdline'] = cmdlineConfig;
  inheritConfiguration(settings.configurations,
                       '@cmdline', settings.properties);
  if (!command || command == 'help') {
    console.log('usage: campfire command, arguments');
    console.log('');
    console.log('the following commands are available:');
    for (var availableCommand in commands.commands) {
      console.log('\t' + availableCommand + ':\t' +
          commands.commands[availableCommand].help);
    }
  } else {
    var resolvedCommand = commands.commands[command];
    if (!resolvedCommand) {
      console.error('Unknown command: ' + command);
      process.exit(1);
    }
    resolvedCommand.run(new engine.Engine(settings, campfireRoot, relative),
                        targets);
  }
}

engine.acquireLock(runCampfire);
