// We can export objects to other modules.
// Note that var quux = {}; module.exports = quux; appears to be broken.
//- @exports defines/binding Export
module.exports = {
  //- @hello defines/binding ExportedHello
  hello: "string"
}
