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

// Repeat same namespace to make
// sure we don't output duplicate
// facts.
//- SecondNdef defines NValue
//- SecondNdef.loc/start @^"namespace"
namespace N {
  export const pinky = 1;
  //- SecondNdef.loc/end @$"}"
}

//- @Z defines/binding ZNamespace
//- ZNamespace.node/kind record
//- ZNamespace.subkind namespace
//- @Z defines/binding ZValue
//- ZValue.node/kind package
//- Zdef defines ZValue
//- Zdef.loc/start @^"Z"
//- ThirdNDef defines NValue
//- ThirdNDef.loc/start @^"namespace"
namespace N.Z {
  export const blinky = 2;
  //- Zdef.loc/end @$"}"
  //- ThirdNDef.loc/end @$"}"
}
