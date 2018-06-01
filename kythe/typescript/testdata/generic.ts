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

// Create a generic type and instantiate it.
//- @Container defines/binding Container
//- @T defines/binding ContainerT
//- T.node/kind absvar
interface Container<T> {
  //- @T ref ContainerT
  //- !{@T ref Container}
  contained: T;
}
let box: Container<IFace>;

//- @#0T defines/binding FnT
//- FnT.node/kind absvar
//- @IFace ref IFace
function generic<T>(x: T, y: IFace) {
  return x;
}

//- @Container ref Container
//- @IFace ref Iface
interface ExtendsGeneric extends Container<IFace> {}

//- @Container ref Container
//- @IFace ref Iface
class ImplementsGeneric implements Container<IFace> {
  //- @IFace ref Iface
  contained: IFace;
}
