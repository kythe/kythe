// Checks that we record implicit anchors for init lists.
//- DCtor childof ClassD
//- @D defines/binding ClassD
//- DCtor.subkind constructor
//- DCtor.complete definition
class D { };

//- ECtor childof ClassE
//- @E defines/binding ClassE
//- ECtor.subkind constructor
//- ECtor.complete definition
class E { };

//- @Class defines/binding Class
//- CCtor childof Class
//- CCtor.subkind constructor
//- CCtor.complete definition
class Class {
  D d;
  E e;
};

Class c;

//- Call1 childof CCtor
//- Call1.subkind implicit
//- Call1 ref/call DCtor

//- Call2 childof CCtor
//- Call2.subkind implicit
//- Call2 ref/call ECtor
