//- @"\"depone\"" ref/includes FileOne=vname("",depone,"","index.js",_)
var depone = require("depone")
//- @"\"deptwo\"" ref/includes FileTwo=vname("",deptwo,"","main.js",_)
var deptwo = require("deptwo")
//- @getName ref/call GetNameOne=vname(_,depone,_,_,_)
var nameOne = depone.getName();
//- @getName ref/call GetNameTwo=vname(_,deptwo,_,_,_)
var nameTwo = deptwo.getName();
