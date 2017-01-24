#define __header_inline inline

__header_inline int
isalnum(int _c)
{
      return 0;
}

int main(int argc, char **argv) {
  //- @isalnum ref BuiltinFn
  //- @isalnum ref vname("isalnum#n#builtin", "", "", "", "objectivec")
  //- ImplicitLoc=vname("isalnum#n#builtin@syntactic", "", "", "", "objectivec")
  //-   defines/binding BuiltinFn
  //- !{ ImplicitLoc.loc/start _
  //-    ImplicitLoc.loc/end _ }
  isalnum(1);

  return 0;
}
