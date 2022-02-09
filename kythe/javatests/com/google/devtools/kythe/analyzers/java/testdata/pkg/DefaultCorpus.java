package pkg;

import java.util.List;

// Test that when the --use_compilation_corpus_as_default option is enabled,
// tapps are placed in the CU's corpus. In this example, the "List<String>" tapp
// should be given the "kythe" corpus.

class DefaultCorpus {
  //- @myvar defines/binding MyVar?
  //- MyVar typed ListStringType=vname(_,"kythe",_,_,"java")
  //- ListStringType.node/kind "tapp"
  private List<String> myvar;

  private List<? extends Object> objList;

  //- @myMethod defines/binding MyMethod
  //- MyMethod typed MyMethodType=vname(_,"kythe",_,_,"java")
  //- MyMethodType.node/kind "tapp"
  //- MyMethodType param.0 FnBuiltin?=vname("fn#builtin","kythe",_,_,"java")
  String myMethod() {
    return "";
  }
}
