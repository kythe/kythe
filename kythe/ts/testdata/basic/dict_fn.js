// Checks that we emit ref/call edges for functions defined in dictionaries.
var dict = {
//- @g defines/binding VarG
//- VarG.node/kind variable
  g: function(x) { return x; }
};
//- @g ref/call VarG
var v = dict.g(1);
