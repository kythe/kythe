'use strict';

exports.Node = function() {
};

exports.Node.prototype.init = function(id) {
  this.id = id;
  this.inputs = [];
  this.outputs = [];
};

exports.Node.prototype.addParent = function(parent) {
  this.inputs.push(parent);
  parent.outputs.push(this);
};

exports.Graph = function() {
  this.nodes = [];
  this.ids = {};
};

exports.Graph.prototype.addNode = function(node) {
  if (this.ids[node.id]) {
    console.trace('ERROR: graph already contains node: ' + node.id);
    process.exit(2);
  }
  this.nodes.push(node);
  this.ids[node.id] = node;
  return node;
};

exports.Graph.prototype.getNode = function(id) {
  return this.ids[id];
};

exports.Graph.prototype.stronglyConnectedComponents = function() {
  var index = 0;
  var stack = [];
  var stackPresent = {};
  var indexes = {};
  var lowlinks = {};
  var components = [];

  function strongconnect(node) {
    indexes[node.id] = index;
    lowlinks[node.id] = index;
    index++;
    stack.push(node);
    stackPresent[node.id] = true;

    for (var i = 0; i < node.outputs.length; i++) {
      var parent = node.outputs[i];
      if (indexes[parent.id] === undefined) {
        strongconnect(parent);
        lowlinks[node.id] = Math.min(lowlinks[node.id],
                                     lowlinks[parent.id]);
      } else if (stackPresent[parent.id] === true) {
        lowlinks[node.id] = Math.min(lowlinks[node.id],
                                     indexes[parent.id]);
      }
    }

    if (lowlinks[node.id] === indexes[node.id]) {
      var component = [];
      while (true) {
        var current = stack.pop();
        stackPresent[current] = undefined;
        component.push(current);
        if (current === node) {
          break;
        }
      }
      if (component.length > 1) {
        components.push(component);
      }
    }
  }

  for (var i = 0; i < this.nodes.length; i++) {
    var node = this.nodes[i];
    if (indexes[node.id] === undefined) {
      strongconnect(node);
    }
  }
  return components;
};
