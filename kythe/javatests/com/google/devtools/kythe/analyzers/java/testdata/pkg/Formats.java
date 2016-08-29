// http://www.kythe.io/docs/schema/#formats

//- @pkg ref Package
//- Package.format pkg
package pkg;

//- @Formats defines/binding Class
//- Class.format "%[^.%]%[iFormats%]"
//- Class childof Package
public class Formats {

  //- @fieldName defines/binding Field
  //- Field.format "%[^.%]%[ifieldName%]"
  //- Field childof Class
  //- @int ref Int
  int fieldName;

  //- @methodName defines/binding Method
  //- Method.format "%[^.%]%[imethodName%]%[p(%0,)%]"
  //- Method childof Class
  //- Method typed MType
  //- MType param.1 Void
  void methodName() {}

  //- @methodWithParams defines/binding MethodWithParams
  //- MethodWithParams.format "%[^.%]%[imethodWithParams%]%[p(%0,)%]"
  //- MethodWithParams typed MPType
  //- MPType param.1 Void
  //- MPType param.2 String
  //- MPType param.3 Int
  void methodWithParams(String a, int b) {}

  //- Void.format void
  //- Int.format int
  //- String.format "%[^.%]%[iString%]"
}
