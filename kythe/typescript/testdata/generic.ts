export {}

// Check references to a type in a generic.

//- @IFace defines/binding IFace
//- IFace.node/kind interface
interface IFace {
  foo: string;
}

// Reference to IFace in a generic type.
//- @IFace ref IFace
let x: Map<string, IFace>;
// Reference to IFace in an expression.
//- @IFace ref IFace
x = new Map<string, IFace>();
