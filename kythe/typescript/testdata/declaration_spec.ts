// Tests the specification of VNames for symbol declarations

// SourceFile
//- FileModule=VName("module", _, _, "testdata/declaration_spec", "typescript").node/kind record
//- FileModuleAnchor.node/kind anchor
//- FileModuleAnchor./kythe/loc/start 0
//- FileModuleAnchor./kythe/loc/end 1
//- FileModuleAnchor defines/binding FileModule

// NamespaceImport
//- @NspI defines/binding VName("NspI", _, _, "testdata/declaration_spec", "typescript")
import * as NspI from './declaration';

// ExportAssignment
//- @default defines/binding VName("default", _, _, "testdata/declaration_spec", "typescript")
export default NspI;

// ClassDeclaration
//- @C defines/binding VName("C#type", _, _, "testdata/declaration_spec", "typescript")
//- @C defines/binding VName("C", _, _, "testdata/declaration_spec", "typescript")
class C {
  // PropertyDeclaration instance member
  //- @property defines/binding VName("C#type.property", _, _, "testdata/declaration_spec", "typescript")
  property = 0;

  // PropertyDeclaration static member
  //- @property defines/binding VName("C.property", _, _, "testdata/declaration_spec", "typescript")
  static property = 0;

  // MethodDeclaration
  //- @method defines/binding VName("C#type.method", _, _, "testdata/declaration_spec", "typescript")
  method() {}

  // Constructor, ParameterPropertyDeclaration
  //- @constructor defines/binding VName("C#type.constructor", _, _, "testdata/declaration_spec", "typescript")
  //- @cprop defines/binding VName("C#type.cprop", _, _, "testdata/declaration_spec", "typescript")
  constructor(private cprop: number) {}

  // GetAccessor
  //- @prop defines/binding VName("C#type.prop:getter", _, _, "testdata/declaration_spec", "typescript")
  //- @prop defines/binding VName("C#type.prop", _, _, "testdata/declaration_spec", "typescript")
  get prop() {
    return this.property;
  }

  // SetAccessor
  //- @prop defines/binding VName("C#type.prop:setter", _, _, "testdata/declaration_spec", "typescript")
  set prop(nProp) {
    this.property = nProp;
  }
}

// ClassDeclaration with no ctor
//- @CC defines/binding VName("CC#type", _, _, "testdata/declaration_spec", "typescript")
//- @CC defines/binding VName("CC", _, _, "testdata/declaration_spec", "typescript")
class CC {}

// EnumDeclaration
//- @E defines/binding VName("E", _, _, "testdata/declaration_spec", "typescript")
//- @E defines/binding VName("E#type", _, _, "testdata/declaration_spec", "typescript")
enum E {
  // EnumMember
  //- @EnumMember defines/binding VName("E.EnumMember", _, _, "testdata/declaration_spec", "typescript")
  EnumMember = 0
}

// FunctionDeclaration
//- @#1"fun" defines/binding VName("fun", _, _, "testdata/declaration_spec", "typescript")
function fun(
    // Parameter
    //- @param defines/binding VName("fun.param", _, _, "testdata/declaration_spec", "typescript")
    param: number) {}

// InterfaceDeclaration
//- @B defines/binding VName("B#type", _, _, "testdata/declaration_spec", "typescript")
interface B {
  // PropertySignature
  //- @pSig defines/binding VName("B.pSig", _, _, "testdata/declaration_spec", "typescript")
  pSig: number;

  // MethodSignature
  //- @mSig defines/binding VName("B.mSig", _, _, "testdata/declaration_spec", "typescript")
  mSig(): void;
}

// VariableDeclaration
//- @v defines/binding VName("v", _, _, "testdata/declaration_spec", "typescript")
let v = {
  // PropertyAssignment
  // TODO: the signature here should be something like `block0.prop`, but
  // anonymous block names are not well-defined by the spec yet.
  //- @prop defines/binding VName(_, _, _, "testdata/declaration_spec", "typescript")
  prop: 0
};

// TypeAliasDeclaration
//- @AliasArray defines/binding VName("AliasArray#type", _, _, "testdata/declaration_spec", "typescript")
type AliasArray<
    // TypeParameter
    //- @#0"T" defines/binding VName("AliasArray.T#type", _, _, "testdata/declaration_spec", "typescript")
    T> = Array<T>;

//- @arrowFun defines/binding VName("arrowFun", _, _, "testdata/declaration_spec", "typescript")
const arrowFun = () => {
  // Arrow function scope name is not well-defined.
  //- @anonArrowFunDecl defines/binding VName(_, _, _, "testdata/declaration_spec", "typescript")
  let anonArrowFunDecl;
};

{
  // Anonymous block scope name is not well-defined.
  //- @anonBlockDecl defines/binding VName(_, _, _, "testdata/declaration_spec", "typescript")
  let anonBlockDecl;
}
