'use strict';

exports.Entity = function(id) {
  this.init(id);
};
exports.Entity.prototype.init = function(id) {
  this.id = id;
};
exports.Entity.prototype.getFileInputs = function() {
  var files = [];
  for (var i = 0; i < this.inputs.length; i++) {
    if (this.inputs[i].__proto__ == exports.File.prototype) {
      files.push(this.inputs[i].getPath());
    }
  }
  return files;
};

exports.File = function(id, kind, owner) {
  this.init(id);
  this.kind = kind;
  this.owner = owner;
};
exports.File.prototype = new exports.Entity();
exports.File.prototype.getPath = function() {
  return this.id;
};

exports.Directory = function(id) {
  this.init(id);
};
exports.Directory.prototype = new exports.Entity();
exports.Directory.prototype.getPath = function() {
  return this.id;
};

exports.Property = function(ownerId, key, value) {
  this.init(ownerId + ':' + key);
  this.key = key;
  this.value = value;
};
exports.Property.prototype = new exports.Entity();
