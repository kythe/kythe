export {}

interface HasIterator {
  // Just verify we index into the computed property:
  //- @Symbol ref Sym=VName("typescript/lib/lib.es5/Symbol", _, _, _, _)
  //- @iterator ref _Iter=VName("typescript/lib/lib.es2015.iterable/SymbolConstructor.iterator", _, _, _, _)
  [Symbol.iterator](): number;
}

//- @mySymbol defines/binding My
const mySymbol = Symbol('my');
let x = {
  //- @mySymbol ref My
  [mySymbol]() {
    //- @Symbol ref Sym
    Symbol;
  }
}
