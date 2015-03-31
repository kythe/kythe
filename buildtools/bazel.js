'use strict';

var fs = require('fs');
var path = require('path');
var yaml = require('js-yaml');

exports.Converter = function(engine) {
  this.engine = engine;
  var configPath = engine.settings.properties['bazel_config'];
  if (!configPath) {
    console.error('ERROR: missing --bazel_config=<path>');
    process.exit(1);
  }
  console.warn('Loading Bazel configuration from "' + configPath + '"');
  var config = yaml.safeLoad(fs.readFileSync(configPath));
  this.translations = config['translations'];
  this.rules = config['rules'];
};

exports.Converter.prototype.convertTargets = function(targets) {
  if (targets.length === 0) {
    targets.push('//...');
  }
  for (var i = 0; i < targets.length; i++) {
    this.engine.resolveTargets(targets[i]);
  }

  var writtenBuildFiles = {};
  var self = this;
  this.engine.targets.nodes.map(function(target) {
    for (var i = 0; i < self.translations.exclude.length; i++) {
      if (target.id.startsWith(self.translations.exclude[i])) {
        return;
      }
    }
    var convs = self.rules[target.rule.config_name];
    if (convs) {
      if (!(convs instanceof Array)) {
        convs = [convs];
      }
      for (var i = 0; i < convs.length; i++) {
        var file = path.join(path.dirname(target.asPath()), 'BUILD');
        var bazelTarget = self.convertTarget(convs[i], target);
        var buf = new Buffer(displayTarget(bazelTarget));
        if (writtenBuildFiles[file]) {
          fs.appendFileSync(file, buf);
        } else {
          fs.writeFileSync(file, buf);
          writtenBuildFiles[file] = true;
        }
      }
      console.log(target.id);
    }
  });
};

exports.Converter.prototype.convertTarget = function(conv, target) {
  var res = {name: target.json.name};
  for (var label in conv) {
    var val = this.convertValue(conv[label], target);
    if (val) {
      res[label] = val;
    }
  }
  return res;
}

exports.Converter.prototype.convertValue = function(val, target) {
  switch (typeof(val)) {
    case 'string': {
      if (val.startsWith('*')) {
        var res = this.convertValue(val.substring(1), target);
        if (res instanceof Array && res.length == 1) {
          return res[0];
        }
        return res;
      } else if (val.startsWith('#')) {
        var accessors = val.substring(1).split('.');
        var res = target;
        for (var i = 0; i < accessors.length && res; i++) {
          res = res[accessors[i]];
        }
        return this.convertTargetValues(target, res);
      }
      return val;
    }
    case 'number': return val;
  }
  if (val instanceof Array) {
    var res = [];
    for (var i = 0; i < val.length; i++) {
      res.push(this.convertValue(val[i]));
    }
    return res;
  }
  console.error('Unknown conversion value (' + typeof(val) + '): ' + val);
  process.exit(1);
}

exports.Converter.prototype.convertTargetValues = function(context, targets) {
  if (!(targets instanceof Array)) {
    return targets;
  }
  var contextDir = path.dirname(context.asPath());
  var res = [];
  for (var i = 0; i < targets.length; i++) {
    var id = typeof(targets[i]) === 'string' ? targets[i] : targets[i].id;
    if (!id.startsWith('//')) {
      res.push(path.relative(contextDir, targets[i].asPath()));
    } else if (this.translations.direct[id]) {
      var trans = this.translations.direct[id];
      if (trans instanceof Array) {
        res.append(this.convertTargetValues(context, trans));
      } else {
        res.append(this.convertTargetValues(context, [trans]));
      }
    } else {
      var packageDir = id.substring(2, id.indexOf(':'));
      if (packageDir === contextDir) {
        res.push(id.substring(id.indexOf(':')));
      } else {
        for (var prefix in this.translations.prefix) {
          if (id.startsWith(prefix)) {
            id = this.translations.prefix[prefix] + id.substring(prefix.length);
          }
        }
        res.push(id);
      }
    }
  }
  return res;
}

function displayTarget(target) {
  var res = '';
  if (target.builddefs) {
    for (var i = 0; i < target.builddefs.length; i++) {
      res += 'subinclude("' + target.builddefs[i] + '")\n';
    }
  }
  res += target.rule + '(\n';
  for (var label in target) {
    if (label === 'rule' || label === 'builddefs') {
      continue;
    }
    res += '    ' + label + ' = ';
    res += displayValue(target[label]) + ',\n';
  }
  return res + ')\n';
}

function displayValue(val) {
  if (typeof(val) == 'string') {
    return '"' + val + '"';
  } else if (typeof(val) == 'number') {
    return val;
  } else if (val instanceof Array) {
    var res = '[';
    for (var i = 0; i < val.length; i++) {
      res += '\n        ';
      res += displayValue(val[i]) + ',';
    }
    if (val.length > 0) {
      res += '\n';
    }
    return res + '    ]';
  } else if (val.value) {
    return displayValue(val.value);
  }
  console.error('Unhandled value: ' + val);
  process.exit(1);
}
