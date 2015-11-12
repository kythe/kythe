// We can require objects exported by other modules.
//- @required defines/binding RequiredVariable
//- @"\"./export_object\"" ref/includes ExportingFile
var required = require("./export_object");
//- @required ref RequiredVariable
//- @hello ref ExportedHello
var hi = required.hello
//- ExportedBinding defines/binding ExportedHello
//- ExportedBinding childof ExportingFile
