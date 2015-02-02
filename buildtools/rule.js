'use strict';

var fs = require('fs');
var path = require('path');

var graphs = require('./graphs.js');
var entity = require('./entity.js');

exports.Tool = function(engine, name,
                        tool, args,
                        regexp, minversion) {
  this.engine = engine;
  this.out = new entity.File(path.join('campfire-out/tools/', name),
                             'version_marker', this);
  this.tool = tool;
  this.args = args;
  this.regexp = regexp;
  this.minversion = minversion;
};
exports.Tool.prototype.getBuild = function() {
  if (this.engine.settings.properties['skip_version_checks']) {
    return {
      rule: 'touch',
      inputs: [],
      outs: [this.out]
    };
  }
  return {
    rule: 'check_version',
    inputs: [],
    outs: [this.out],
    vars: {
      tool: this.tool,
      args: this.args.join(' '),
      regexp: this.regexp,
      minversion: this.minversion
    }
  };
};

exports.Target = function(id, json) {
  this.init(id);
  this.json = json;
};

exports.Target.prototype = new graphs.Node();
exports.Target.prototype.asPath = function() {
  return this.id.substring(2).replace(':', '/');
};

exports.Target.prototype.getRoot = function(kind) {
  var out_label = this.engine.settings.properties['out_label'];
  if (out_label === '')
  return path.join('campfire-out/', kind, this.asPath());
  else
  return path.join('campfire-out/', out_label, kind, this.asPath());
};

exports.Target.prototype.baseName = function() {
  var index = this.id.indexOf(':');
  return this.id.substring(index + 1);
};

exports.Target.prototype.getProperty = function(name) {
  if (this.properties && this.properties[name]) {
    return this.properties[name];
  } else {
    if (this.engine.settings.properties &&
        this.engine.settings.properties[name]) {
      var prop = new entity.Property(this.id, name,
                                     this.engine.settings.properties[name]);
      var loaded = this.engine.entities[prop.id];
      if (loaded) {
        return loaded;
      }
      this.engine.entities[prop.id] = prop;
      return prop;
    }
  }
  return undefined;
};

exports.Target.prototype.getFileNode = function(path, kind) {
  var file = this.engine.entities[path];
  if (file) {
    if (!(file instanceof entity.File)) {
      console.trace('Mismatching entity kind for ' + path);
      process.exit(1);
    } else if (file.kind !== kind) {
      console.trace('Mismatching file kinds for ' + path +
          ' (existing "' + file.kind + '"; wanted "' + kind + '")');
      process.exit(1);
    } else if (file.owner !== this) {
      console.trace('Mismatching file owners for ' + path +
          ' (existing "' + file.owner.id + '"; wanted "' + this.id + '")');
      process.exit(1);
    }
    return file;
  }
  file = new entity.File(path, kind, this);
  this.engine.entities[path] = file;
  return file;
};

exports.Target.prototype.getDirectoryNode = function(path) {
  var dir = this.engine.entities[path];
  if (dir) {
    if (!(dir instanceof entity.Directory)) {
      console.trace('Mismatching entity kind for ' + path);
      process.exit(1);
    }
    return dir;
  }
  dir = new entity.Directory(path);
  this.engine.entities[path] = dir;
  return dir;
};

exports.Target.prototype.getPropertyValue = function(name) {
  var prop = this.getProperty(name);
  return prop ? prop.value : undefined;
};

exports.Target.prototype.getVersionMarker = function(name) {
  var tool = this.engine.tools[name];
  if (!tool) {
    console.error('ERROR: no such tool named "' + name + '"');
    process.exit(1);
  }
  return tool.out;
};

exports.Rule = function(engine) {
  this.engine = engine;
};

exports.Rule.prototype.createTarget = function(name, id, inputsByKind,
                                               properties, json) {
  var node = new exports.Target(id, json);
  this.engine.targets.addNode(node);
  node.inputsByKind = inputsByKind;
  for (var kind in inputsByKind) {
    var inputs = inputsByKind[kind];
    for (var i = 0; i < inputs.length; i++) {
      node.addParent(inputs[i]);
    }
  }
  node.properties = properties;
  node.rule = this;
  node.engine = this.engine;
  node.name = name;
  return node;
};

exports.Rule.prototype.getExecutable = function(target) {
  return null;
};

exports.Rule.prototype.getOutputsFor = function(target, kind) {
  kind = kind.name ? kind : exports.kinds[kind.toUpperCase()];
  return this.getBuilds(target, kind)
      .map(function(b) { return (b.outs || []).concat(b.properties || []); })
      .reduce(function(lst, outs) { return lst.concat(outs); }, []);
};

exports.Rule.prototype.getBuilds = function(target, kind) {
  if (!this.getNinjaBuilds) {
    return [];
  } else if (!target.cachedBuilds) {
    target.cachedBuilds = this.getNinjaBuilds(target);
    if (target.cachedBuilds instanceof Array) {
      target.cachedBuilds = {BUILD: target.cachedBuilds};
    }
  }
  if (!kind) {
    return [];
  }
  return kind.extractBuilds(target.cachedBuilds);
};

exports.StaticFile = function(engine) {
  this.engine = engine;
};

exports.StaticFile.prototype = new exports.Rule();
exports.StaticFile.prototype.getBuilds = undefined;
exports.StaticFile.prototype.getOutputsFor = function(target, kind) {
  if (target.outs) {
    return target.outs;
  }
  var fileName = target.id;
  if (fileName.indexOf('//') === 0) {
    fileName = fileName.substring(2).replace(':', '/');
  }
  if (fs.lstatSync(fileName).isDirectory()) {
    target.outs = [target.getDirectoryNode(fileName)];
  } else {
    target.outs = [target.getFileNode(fileName, 'src_file')];
  }
  return target.outs;
};

exports.getAllOutputsRecursiveFor = function(targets, kind, filter,
                                             results, visited) {
  if (!targets) {
    return [];
  }
  if (!results) {
    results = [];
    exports.getAllOutputsRecursiveFor(
        targets, kind, filter, results, {});
    return results;
  }
  for (var i = 0; i < targets.length; i++) {
    var target = targets[i];
    if (visited[target.id]) {
      continue;
    }
    visited[target.id] = true;
    var outputs = target.rule.getOutputsFor(target, kind);
    for (var j = 0; j < outputs.length; j++) {
      if (filter) {
        if (!filter(outputs[j])) {
          continue;
        }
      }
      results.push(outputs[j]);
    }
    exports.getAllOutputsRecursiveFor(target.inputs, kind, filter,
                                      results, visited);
  }
};

exports.getAllOutputsFor = function(targets, kind, filter) {
  var results = [];
  if (!targets) {
    return results;
  }
  for (var i = 0; i < targets.length; i++) {
    var target = targets[i];
    var outputs = target.rule.getOutputsFor(target, kind);
    for (var j = 0; j < outputs.length; j++) {
      if (filter) {
        if (!filter(outputs[j])) {
          continue;
        }
      }
      results.push(outputs[j]);
    }
  }
  return results;
};

exports.fileFilter = function(kind, extension) {
  return function(output) {
    if (output.__proto__ == entity.File.prototype &&
        output.kind == kind) {
      if (extension) {
        return entity.File.prototype &&
            output.getPath().endsWith(extension);
      }
      return true;
    }
    return false;
  };
};

exports.propertyFilter = function(name) {
  return function(output) {
    return output.__proto__ == entity.Property.prototype &&
        output.key == name;
  };
};

exports.allFilesFilter = function(output) {
  return output.__proto__ == entity.File.prototype;
};

exports.allFilesAndDirsFilter = function(output) {
  return exports.allFilesFilter(output) ||
      output.__proto__ == entity.Directory.prototype;
};

exports.getPaths = function(files, prefix) {
  return files.map(function(f) {
    var p = f.getPath ? f.getPath() : f;
    return prefix ? path.join(prefix, p) : p;
  });
};

exports.getExecutable = function(engine, target) {
  target = typeof(target) == 'object' ? target : engine.resolveTarget(target);
  return target.rule.getExecutable(target);
};

function Kind(name, implies) {
  this.name = name;
  var set = (implies || [])
      .map(function(k) { return k.implies; })
      .reduce(function(set, k) {
        for (var i = 0; i < k.length; i++) {
          set[k[i]] = true;
        }
        return set;
      }, {});
  set[name] = true;
  this.implies = Object.keys(set);
}
Kind.prototype.extractBuilds = function(builds) {
  return this.implies
      .map(function(k) { return builds[k] || []; })
      .reduce(function(builds, bs) { return builds.concat(bs); }, []);
};

exports.kinds = {};
exports.kinds.BUILD = new Kind('BUILD');
exports.kinds.TEST = new Kind('TEST', [exports.kinds.BUILD]);
exports.kinds.EXTRACT = new Kind('EXTRACT', [exports.kinds.BUILD]);
exports.kinds.BENCH = new Kind('BENCH', [exports.kinds.BUILD]);
exports.kinds.PACKAGE = new Kind('PACKAGE', [exports.kinds.BUILD]);
exports.kinds.DEPLOY = new Kind('DEPLOY', [exports.kinds.PACKAGE]);
