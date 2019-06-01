// Tests the specification of VNames for symbol declarations

// NamespaceImport
//- @NspI defines/binding VName("NspI", _, _, "testdata/spec", "typescript")
import * as NspI from "./declaration";

// ImportSpecifier
//- @I defines/binding VName("I", _, _, "testdata/spec", "typescript")
import { decl as I } from "./declaration";

// ExportSpecifier
//- @exp define/binding VName("exp", _, _, "testdata/spec", "typescript")
export { E as exp };

// ModuleDeclaration
//- @N define/binding VName("N", _, _, "testdata/spec", "typescript")
namespace N {}

// ClassDeclaration
//- @C defines/binding VName("C", _, _, "testdata/spec", "typescript")
class C {
  // PropertyDeclaration
  //- @method defines/binding VName("C.property", _, _, "testdata/spec", "typescript")
  property = 0;

  // MethodDeclaration
  //- @method defines/binding VName("C.method", _, _, "testdata/spec", "typescript")
  method() {}
}

// EnumDeclaration
//- @D defines/binding VName("E", _, _, "testdata/spec", "typescript")
enum E {
  // EnumMember
  //- @EnumMember defines/binding VName("E.EnumMember", _, _, "testdata/spec", "typescript")
  EnumMember = 0
}

// FunctionDeclaration
//- @fun defines/binding VName("fun", _, _, "testdata/spec", "typescript")
function fun(
  // Parameter
  //- @param defines/binding VName("fun.param", _, _, "testdata/spec", "typescript")
  param: number
) {}

// InterfaceDeclaration
//- @B defines/binding VName("B", _, _, "testdata/spec", "typescript")
interface B {
  // PropertySignature
  //- @pSig defines/binding VName("B.pSig", _, _, "testdata/spec", "typescript")
  pSig: number;

  // MethodSignature
  //- @mSig defines/binding VName("B.mSig", _, _, "testdata/spec", "typescript")
  mSig(): void;
}

// VariableDeclaration
//- @v defines/binding VName("v", _, _, "testdata/spec", "typescript")
let v = {
  // PropertyAssignment
  //- @prop defines/binding VName("v.prop", _, _, "testdata/spec", "typescript")
  prop: 0
};

// TypeAliasDeclaration
//- @AliasArray defines/binding VName("AliasArray", _, _, "testdata/spec", "typescript")
type AliasArray<
  // TypeParameter
  //- @T defines/binding VName("AliasArray.T", _, _, "testdata/spec", "typescript")
  T
> = Array<T>;

//- @decl defines/binding VName("aFun", _, _, "testdata/spec", "typescript")
const aFun = () => {
  // Arrow function scope name is not well-defined.
  //- @decl defines/binding VName(_, _, _, "testdata/spec", "typescript")
  let anonArrowFunDecl;
};

{
  // Anonymous block scope name is not well-defined.
  //- @decl defines/binding VName(_, _, _, "testdata/spec", "typescript")
  let anonBlockDecl;
}
