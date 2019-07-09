// Test indexing support for namespaces.

//- @N defines/binding NNamespace
//- NNamespace.node/kind record
//- NNamespace.subkind namespace
//- NNamespace.complete definition
//- @N defines/binding NValue
//- NValue.node/kind package
//- Ndef defines NValue
//- Ndef.loc/start @^"namespace"
namespace N {
  //- @obj defines/binding NObj
  export const obj = {key: 'value'};
  //- Ndef.loc/end @$"}"
}

//- @N ref NValue
//- @obj ref NObj
N.obj;
