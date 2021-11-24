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
//- ContainerT.node/kind tvar
//- Container tparam.0 ContainerT
interface Container<T> {
  //- @T ref ContainerT
  //- !{@T ref Container}
  contained: T;
}
let box: Container<IFace>;

//- @#0T defines/binding FnT
//- FnT.node/kind tvar
//- @generic defines/binding FnGeneric
//- FnGeneric tparam.0 FnT
//- @IFace ref IFace
function generic<T>(x: T, y: IFace) {
  return x;
}

// Simple constrained generic.
//- @T defines/binding ConstrainedGenericT
//- @IFace ref IFace
//- ConstrainedGenericT.node/kind tvar
//- ConstrainedGenericT bounded/upper IFace
function constrainedGeneric<T extends IFace>() {}

// Constrained generic with reference to type param within it.
//- @constrainedGenericRef defines/binding FnconstrainedGenericRef
//- FnconstrainedGenericRef tparam.0 ConstrainedGenericT2
//- FnconstrainedGenericRef tparam.1 ConstrainedGenericT2K
//- @#0T defines/binding ConstrainedGenericT2
//- !{@#0T ref ConstrainedGenericT}
//- @#1T ref ConstrainedGenericT2
//- @#0K defines/binding ConstrainedGenericT2K
//- @#1K ref ConstrainedGenericT2K
function constrainedGenericRef<T, K extends keyof T>(k: K) {}

// Recursive generic type.
//- @constrainedGenericRecursive defines/binding FncGR
//- FncGR tparam.0 ConstrainedGenericT3
//- @#0T defines/binding ConstrainedGenericT3
//- @#1T ref ConstrainedGenericT3
function constrainedGenericRecursive<T extends Array<T>>() {}

// Default generic.
//- @defaultGeneric defines/binding FndefaultGeneric
//- FndefaultGeneric tparam.0 DefaultGeneric
//- @#0T defines/binding DefaultGeneric
//- @#1T ref DefaultGeneric
//- @IFace ref IFace
function defaultGeneric<T = IFace>(t: T) {}

// Default generic with extends.
//- @defaultGeneric2 defines/binding FndefaultGeneric2
//- FndefaultGeneric2 tparam.0 DefaultGeneric2
//- @#0T defines/binding DefaultGeneric2
//- @#1T ref DefaultGeneric2
//- @#0IFace ref IFace
//- @#1IFace ref IFace
function defaultGeneric2<T extends IFace = IFace>(t: T) {}

//- @Container ref Container
//- @IFace ref Iface
interface ExtendsGeneric extends Container<IFace> {}

//- @Container ref Container
//- @IFace ref Iface
class ImplementsGeneric implements Container<IFace> {
  //- @IFace ref Iface
  contained: IFace;
}
