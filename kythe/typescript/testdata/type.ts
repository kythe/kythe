export {}

// Declare the name 'typeAndValue' as both a type and a value,
// to ensure we keep the type/value namespaces separate.

//- @typeAndValue defines/binding Value
//- Value.node/kind variable
let typeAndValue: number;

//- @typeAndValue defines/binding Type
//- Type.node/kind talias
type typeAndValue = string;
// TODO: check that 'string' refers to the appropriate type as well.

//- @typeAndValue ref Value
//- !{@typeAndValue ref Type}
let x = typeAndValue;
//- @typeAndValue ref Type
//- !{@typeAndValue ref Value}
let y: typeAndValue;
