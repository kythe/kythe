// Checks that we emit ref/call edges for functions defined in dictionaries.
var dict = {
//- @g defines/binding VarG
//- VarG.node/kind variable
//- VarG callableas CG
  g: function(x) { return x; }
};
//- @g ref/call CG
var v = dict.g(1);
