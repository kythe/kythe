'use strict';

var fs = require('fs');
var path = require('path');

/**
 * Pushes each element in array argument into {@code this}.  Returns
 * {@code this}.
 */
Array.prototype.append = function(array) {
  this.push.apply(this, array);
  return this;
};

/**
 * Maps the given function onto {@code this} and returns the concatenation of
 * the results, each assumed to be an array.
 */
Array.prototype.mapcat = function(f) {
  return this.map(f).reduce(function(p, n) { return p.concat(n); }, []);
};

/**
 * Determines if {@code this} begins with the given string.
 */
String.prototype.startsWith = function(prefix) {
  return this.indexOf(prefix) === 0;
};

/**
 * Determines if {@code this} ends with the given string.
 */
String.prototype.endsWith = function(end) {
  var endStr = end.toString();
  var index = this.lastIndexOf(endStr);
  return index >= 0 && index == this.length - endStr.length;
};

/**
 * Removes the given directory and all of its contents, recursively.
 */
exports.rmdirs = function(dir) {
  var files = fs.readdirSync(dir);
  for (var i = 0; i < files.length; i++) {
    var file = path.join(dir, files[i]);
    var stat = fs.statSync(file);
    if (stat.isDirectory()) {
      exports.rmdirs(file);
    } else {
      fs.unlinkSync(file);
    }
  }
  fs.rmdirSync(dir);
};

/**
 * Parses an semantic version number (see http://semver.org) and returns its
 * components in a map with the following keys: 'major', 'minor', and 'patch'.
 */
exports.parseVersion = function(version) {
  var s = version.split('.');
  if (s.length != 3) {
    throw new Error("ERROR parsing release_version: '" + version + "'");
  }
  return {
    major: s[0],
    minor: s[1],
    patch: s[2]
  };
};

/**
 * Merges all of the arguments, assuming they are each an object, into a single
 * new object.  If more than 1 argument contains the same key, precedence is
 * given to later arguments except when they values of a key are both objects.
 * In that case, the values are merged recursively.
 */
exports.merge = function(objects) {
  var obj = {};
  for (var i = 0; i < arguments.length; i++) {
    for (var key in arguments[i]) {
      var val = arguments[i][key];
      if (val instanceof Object && obj[key] instanceof Object) {
        obj[key] = exports.merge(obj[key], val);
      } else {
        obj[key] = val;
      }
    }
  }
  return obj;
};
