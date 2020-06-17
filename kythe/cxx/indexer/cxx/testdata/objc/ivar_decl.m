// Checks that Objective-C instance variables are declared and defined
// correctly and their uses ref the instance variable, not some new
// variable.

//- @Box defines/binding BoxIface
@interface Box {
  //- @data defines/binding Data
  //- Data childof BoxIface
  //- Data.node/kind variable
  //- Data.subkind field
  int data;
}
@end

//- @Box defines/binding BoxImpl
@implementation Box
-(int)foo {
  //- @data ref/writes Data
  self->data = 300;
  return self->data;
}
@end

int main(int argc, char **argv) {
  return 0;
}

