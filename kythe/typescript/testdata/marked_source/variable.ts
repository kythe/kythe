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
