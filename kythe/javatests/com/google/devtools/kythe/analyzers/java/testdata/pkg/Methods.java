package pkg;

//- @IOException ref/imports IOE
import java.io.IOException;

@SuppressWarnings("unused")
//- @Methods defines/binding Class
public class Methods {

  //- ArrayBuiltin=vname("array#builtin","","","","java").node/kind tbuiltin
  //- FnBuiltin=vname("fn#builtin","","","","java").node/kind tbuiltin
  //- IntBuiltin=vname("int#builtin","","","","java").node/kind tbuiltin
  //- VoidBuiltin=vname("void#builtin","","","","java").node/kind tbuiltin

  //- @Methods defines/binding Ctor
  //- @"public Methods() {}" defines Ctor
  //- Ctor.node/kind function
  //- Ctor childof Class
  //- Ctor typed CtorType
  //- CtorType param.0 FnBuiltin
  //- CtorType param.1 Class
  //- CtorType param.2 Class
  public Methods() {}

  //- @Methods ref Class
  //- @create defines/binding Create
  //- Create typed CreateType
  //- CreateType.node/kind tapp
  //- CreateType param.0 FnBuiltin
  //- CreateType param.1 Class
  //- CreateType param.2 VoidBuiltin
  public static Methods create() {
    //- @Methods ref/id Class
    //- @"Methods" ref Ctor
    return new Methods();
  }

  //- @noop defines/binding NoOp
  //- @"private void noop() {}" defines NoOp
  //- NoOp typed NullFunc
  //- NullFunc.node/kind tapp
  //- NullFunc param.0 FnBuiltin
  //- NullFunc param.1 VoidBuiltin
  //- NullFunc param.2 Class
  private void noop() {}

  //- @f defines/binding F
  //- F.node/kind function
  //- F childof Class
  public static int f(int a) {
    return 0;
  }

  //- @f2 defines/binding F2
  //- F2.node/kind function
  //- F2 childof Class
  //- F2 typed StrFunc
  private String f2() {
    return null;
  }

  //- @f3 defines/binding F3
  //- F3 typed StrFunc
  //- StrFunc.node/kind tapp
  //- StrFunc param.0 FnBuiltin
  //- StrFunc param.1 String
  //- StrFunc param.2 Class
  private String f3() {
    return null;
  }

  //- @repeat defines/binding Repeat
  //- Repeat typed StrStrIntFunc
  //- StrStrIntFunc.node/kind tapp
  //- StrStrIntFunc param.0 FnBuiltin
  //- StrStrIntFunc param.1 String
  //- StrStrIntFunc param.2 Class
  //- StrStrIntFunc param.3 String
  //- StrStrIntFunc param.4 IntBuiltin
  private String repeat(String s, int n) {
    StringBuilder b = new StringBuilder();
    for (int i = 0; i < n; i++) {
      b.append(s);
    }
    return b.toString();
  }

  //- @main defines/binding Main
  //- @"String[]" ref StrArray
  //- Main param.0 Args
  //- Args typed StrArray
  //- StrArray.node/kind tapp
  //- StrArray param.0 ArrayBuiltin
  //- StrArray param.1 String
  public static void main(String[] args) {
    //- @#0toString ref ToString
    //- @#1toString ref ToString
    String s = "".toString().toString();
  }

  //- @IOException ref IOE
  public static void throwsException() throws IOException {
    //- @IOException ref/id IOE
    throw new IOException();
  }

  //- @Exception ref _Exception
  public void error() throws Exception {}
}
