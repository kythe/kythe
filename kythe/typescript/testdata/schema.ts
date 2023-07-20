// Tests TypeScript indexer VName schema.

// SourceFile
//- FileModule=VName("module", _, _, "testdata/schema", "typescript").node/kind record
//- FileModuleAnchor.node/kind anchor
//- FileModuleAnchor./kythe/loc/start 0
//- FileModuleAnchor./kythe/loc/end 0
//- FileModuleAnchor defines/implicit FileModule

// NamespaceImport
//- @NspI defines/binding VName("NspI", _, _, "testdata/schema", "typescript")
import * as NspI from './declaration_group/declaration';

// ExportAssignment
//- @default defines/binding VName("default", _, _, "testdata/schema", "typescript")
export default NspI;

// ClassDeclaration
//- @C defines/binding VName("C#type", _, _, "testdata/schema", "typescript")
class C {
  // PropertyDeclaration instance member
  //- @property defines/binding VName("C#type.property", _, _, "testdata/schema", "typescript")
  property = 0;

  // PropertyDeclarartion string literal
  //- @"'propliteral'" defines/binding VName("C#type.\"propliteral\"", _, _, "testdata/schema", "typescript")
  'propliteral' = 0;

  // PropertyDeclaration static member
  //- @property defines/binding VName("C.property", _, _, "testdata/schema", "typescript")
  static property = 0;

  // MethodDeclaration
  //- @method defines/binding VName("C#type.method", _, _, "testdata/schema", "typescript")
  method() {}

  // Constructor, ParameterPropertyDeclaration
  //- @constructor defines/binding VName("C", _, _, "testdata/schema", "typescript")
  //- @cprop defines/binding VName("C#type.cprop", _, _, "testdata/schema", "typescript")
  constructor(private cprop: number) {}

  // GetAccessor
  //- @prop defines/binding VName("C#type.prop:getter", _, _, "testdata/schema", "typescript")
  //- @prop defines/binding VName("C#type.prop", _, _, "testdata/schema", "typescript")
  get prop() {
    return this.property;
  }

  // SetAccessor
  //- @prop defines/binding VName("C#type.prop:setter", _, _, "testdata/schema", "typescript")
  set prop(nProp) {
    this.property = nProp;
  }
}

// ClassDeclaration with no ctor
//- @CC defines/binding VName("CC#type", _, _, "testdata/schema", "typescript")
//- @CC defines/binding VName("CC", _, _, "testdata/schema", "typescript")
class CC {}

// EnumDeclaration
//- @E defines/binding VName("E", _, _, "testdata/schema", "typescript")
//- @E defines/binding VName("E#type", _, _, "testdata/schema", "typescript")
enum E {
  // EnumMember
  //- @EnumMember defines/binding VName("E.EnumMember", _, _, "testdata/schema", "typescript")
  EnumMember = 0
}

// FunctionDeclaration
//- @#1"fun" defines/binding VName("fun", _, _, "testdata/schema", "typescript")
function fun(
    // Parameter
    //- @param defines/binding VName("fun.param", _, _, "testdata/schema", "typescript")
    param: number) {}

// InterfaceDeclaration
//- @B defines/binding VName("B#type", _, _, "testdata/schema", "typescript")
interface B {
  // PropertySignature
  //- @pSig defines/binding VName("B.pSig", _, _, "testdata/schema", "typescript")
  pSig: number;

  // MethodSignature
  //- @mSig defines/binding VName("B.mSig", _, _, "testdata/schema", "typescript")
  mSig(): void;
}

// VariableDeclaration
//- @v defines/binding VName("v", _, _, "testdata/schema", "typescript")
let v = {
  // PropertyAssignment
  // TODO: the signature here should be something like `block0.prop`, but
  // anonymous block names are not well-defined by the spec yet.
  //- @prop defines/binding VName(_, _, _, "testdata/schema", "typescript")
  prop: 0
};

// TypeAliasDeclaration
//- @AliasArray defines/binding VName("AliasArray#type", _, _, "testdata/schema", "typescript")
type AliasArray<
    // TypeParameter
    //- @#0"T" defines/binding VName("AliasArray.T#mtype", _, _, "testdata/schema", "typescript")
    T> = Array<T>;

//- @arrowFun defines/binding VName("arrowFun", _, _, "testdata/schema", "typescript")
const arrowFun = () => {
  // Arrow function scope name is not well-defined.
  //- @anonArrowFunDecl defines/binding VName(_, _, _, "testdata/schema", "typescript")
  let anonArrowFunDecl;
};

{
  // Anonymous block scope name is not well-defined.
  //- @anonBlockDecl defines/binding VName(_, _, _, "testdata/schema", "typescript")
  let anonBlockDecl;
}
