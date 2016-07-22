// http://www.kythe.io/docs/schema/#formats

//- @pkg ref Package
//- Package.format pkg
package pkg;

//- @Formats defines/binding Class
//- Class.format "%^.Formats"
//- Class childof Package
public class Formats {

  //- @fieldName defines/binding Field
  //- Field.format "%^.fieldName"
  //- Field childof Class
  //- @int ref Int
  int fieldName;

  //- @methodName defines/binding Method
  //- Method.format "%1` %^.methodName()"
  //- Method childof Class
  //- Method typed MType
  //- MType param.1 Void
  void methodName() {}

  //- @methodWithParams defines/binding MethodWithParams
  //- MethodWithParams.format "%1` %^.methodWithParams(%2`,%3`)"
  //- MethodWithParams typed MPType
  //- MPType param.1 Void
  //- MPType param.2 String
  //- MPType param.3 Int
  void methodWithParams(String a, int b) {}

  //- Void.format void
  //- Int.format int
  //- String.format "%^.String"
}
