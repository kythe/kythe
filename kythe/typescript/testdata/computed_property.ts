export {}

interface HasIterator {
  // Just verify we index into the computed property:
  //- @Symbol ref Sym=VName("Symbol", _, _, _, _)
  //- @iterator ref _Iter=VName("SymbolConstructor.iterator", _, _, _, _)
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
