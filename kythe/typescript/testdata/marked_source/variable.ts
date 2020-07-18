export {}

//- @a defines/binding A
//- A code ACode
//- ACode child.0 AContext
//- AContext.kind "CONTEXT"
//- AContext.pre_text "const"
//- ACode child.1 ASpace
//- ASpace.kind "BOX"
//- ASpace.pre_text " "
//- ACode child.2 AName
//- AName.kind "IDENTIFIER"
//- AName.pre_text "a"
//- ACode child.3 ATy
//- ATy.kind "TYPE"
//- ATy.pre_text ": "
//- ATy.post_text "string"
//- ACode child.4 AEq
//- AEq.kind "BOX"
//- AEq.pre_text " = "
//- ACode child.5 AInit
//- AInit.kind "INITIALIZER"
//- AInit.pre_text "'text'"
const a: string = 'text';

//- @b defines/binding B
//- B code BCode
//- BCode child.0 BContext
//- BContext.pre_text "const"
//- BCode child.1 BSpace
//- BSpace.pre_text " "
//- BCode child.2 BName
//- BName.pre_text "b"
//- BCode child.3 BTy
//- BTy.post_text "\"text\""
//- BCode child.4 BEq
//- BEq.pre_text " = "
//- BCode child.5 BInit
//- BInit.pre_text "'text'"
const b = 'text';

//- @c defines/binding C
//- C code CCode
//- CCode child.0 CContext
//- CContext.pre_text "let"
//- CCode child.1 CSpace
//- CSpace.pre_text " "
//- CCode child.2 CName
//- CName.pre_text "c"
//- CCode child.3 CTy
//- CTy.post_text "string"
//- CCode child.4 CEq
//- CEq.pre_text " = "
//- CCode child.5 CInit
//- CInit.pre_text "'text'"
let c = 'text';

//- @#0d defines/binding D
//- D code DCode
//- DCode child.0 DContext
//- DContext.pre_text "let"
//- DCode child.1 DSpace
//- DSpace.pre_text " "
//- DCode child.2 DName
//- DName.pre_text "d"
//- DCode child.3 DTy
//- DTy.post_text "string" // not string|undefined b/c tests have strictNullChecks=false
//- !{DCode child.4 _DTy}
let d: string|undefined;

try {
  //- @e defines/binding E
  //- E code ECode
  //- ECode child.0 EContext
  //- EContext.pre_text "(local var)"
  //- ECode child.1 ESpace
  //- ESpace.pre_text " "
  //- ECode child.2 EName
  //- EName.pre_text "e"
  //- ECode child.3 ETy
  //- ETy.post_text "any"
  //- !{ECode child.4 _ETy}
} catch (e) {
}

//- @#0f defines/binding F
//- F code FCode
//- FCode child.0 FContext
//- FContext.pre_text "let"
//- FCode child.1 FSpace
//- FSpace.pre_text " "
//- FCode child.2 FName
//- FName.pre_text "f"
//- FCode child.3 FTy
//- FTy.post_text "number"
//- FCode child.4 FEq
//- FEq.pre_text " = "
//- FCode child.5 FInit
//- FInit.pre_text "0"
//- @halias defines/binding H
//- H code HCode
//- HCode child.0 HContext
//- HContext.pre_text "let"
//- HCode child.1 HSpace
//- HSpace.pre_text " "
//- HCode child.2 HName
//- HName.pre_text "halias"
//- HCode child.3 HTy
//- HTy.post_text "number"
//- HCode child.4 HEq
//- HEq.pre_text " = "
//- HCode child.5 HInit
//- HInit.pre_text "1"
//- @#0redcat defines/binding Redcat
//- Redcat code RedcatCode
//- RedcatCode child.0 RedcatContext
//- RedcatContext.pre_text "let"
//- RedcatCode child.1 RedcatSpace
//- RedcatSpace.pre_text " "
//- RedcatCode child.2 RedcatName
//- RedcatName.pre_text "redcat"
//- RedcatCode child.3 RedcatTy
//- RedcatTy.post_text "number"
//- RedcatCode child.4 RedcatEq
//- RedcatEq.pre_text " = "
//- RedcatCode child.5 RedcatInit
//- RedcatInit.pre_text "2"
let {f, g: {h: halias}, redcat} = {f: 0, g: {h: 1}, ['redcat']: 2};

//- @i defines/binding I
//- I code ICode
//- ICode child.0 IContext
//- IContext.pre_text "let"
//- ICode child.1 ISpace
//- ISpace.pre_text " "
//- ICode child.2 IName
//- IName.pre_text "i"
//- ICode child.3 ITy
//- ITy.post_text "number"
//- ICode child.4 IEq
//- IEq.pre_text " = "
//- ICode child.5 IInit
//- IInit.pre_text "3"
let [_, [i]] = [2, [3]];

// We can't narrow the initializer of j here, so the initializer should be the
// whole RHS of the variable declaration.
//- @#0j defines/binding J
//- J code JCode
//- JCode child.5 JInit
//- JInit.pre_text "(() => {\n  return {j: 1};\n})()"
let {j} = (() => {
  return {j: 1};
})();

//- @k defines/binding K
//- K code KCode
//- KCode child.0 KContext
//- KContext.pre_text "var"
var k = function() {
  //- @l defines/binding L
  //- L code LCode
  //- LCode child.0 LContext
  //- LContext.pre_text "(local var)"
  var l = () => {
    //- @m defines/binding M
    //- M code MCode
    //- MCode child.0 MContext
    //- MContext.pre_text "(local var)"
    var m;
  }
};
