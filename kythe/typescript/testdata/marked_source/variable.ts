export {}

//- @a defines/binding A
//- A code ACode
//- ACode child.0 AName
//- AName.kind "IDENTIFIER"
//- AName.pre_text "a"
//- ACode child.1 ATy
//- ATy.kind "TYPE"
//- ATy.pre_text ": "
//- ATy.post_text "string"
//- ACode child.2 AInit
//- AInit.kind "INITIALIZER"
//- AInit.pre_text "'text'"
const a: string = 'text';

//- @b defines/binding B
//- B code BCode
//- BCode child.0 BName
//- BName.pre_text "b"
//- BCode child.1 BTy
//- BTy.post_text "\"text\""
//- BCode child.2 BInit
//- BInit.pre_text "'text'"
const b = 'text';

//- @c defines/binding C
//- C code CCode
//- CCode child.0 CName
//- CName.pre_text "c"
//- CCode child.1 CTy
//- CTy.post_text "string"
//- CCode child.2 CInit
//- CInit.pre_text "'text'"
let c = 'text';

//- @#0d defines/binding D
//- D code DCode
//- DCode child.0 DName
//- DName.pre_text "d"
//- DCode child.1 DTy
//- DTy.post_text "string" // not string|undefined b/c tests have strictNullChecks=false
//- !{DCode child.2 _DTy}
let d: string|undefined;

try {
  //- @e defines/binding E
  //- E code ECode
  //- ECode child.0 EName
  //- EName.pre_text "e"
  //- ECode child.1 ETy
  //- ETy.post_text "any"
  //- !{ECode child.2 _ETy}
} catch (e) {
}

//- @#0f defines/binding F
//- F code FCode
//- FCode child.0 FName
//- FName.pre_text "f"
//- FCode child.1 FTy
//- FTy.post_text "number"
//- FCode child.2 FInit
//- FInit.pre_text "0"
//- @halias defines/binding H
//- H code HCode
//- HCode child.0 HName
//- HName.pre_text "halias"
//- HCode child.1 HTy
//- HTy.post_text "number"
//- HCode child.2 HInit
//- HInit.pre_text "1"
//- @#0redcat defines/binding Redcat
//- Redcat code RedcatCode
//- RedcatCode child.0 RedcatName
//- RedcatName.pre_text "redcat"
//- RedcatCode child.1 RedcatTy
//- RedcatTy.post_text "number"
//- RedcatCode child.2 RedcatInit
//- RedcatInit.pre_text "2"
let {f, g: {h: halias}, redcat} = {f: 0, g: {h: 1}, ['redcat']: 2};

//- @i defines/binding I
//- I code ICode
//- ICode child.0 IName
//- IName.pre_text "i"
//- ICode child.1 ITy
//- ITy.post_text "number"
//- ICode child.2 IInit
//- IInit.pre_text "3"
let [_, [i]] = [2, [3]];

// We can't narrow the initializer of j here, so the initializer should be the
// whole RHS of the variable declaration.
//- @#0j defines/binding J
//- J code JCode
//- JCode child.2 JInit
//- JInit.pre_text "(() => {\n  return {j: 1};\n})()"
let {j} = (() => {
  return {j: 1};
})();
