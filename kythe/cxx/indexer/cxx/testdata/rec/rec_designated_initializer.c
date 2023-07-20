// We link from designated initializers back to the fields they initialize.

//- @field defines/binding Field
struct s { int field; };
//- @field ref/writes Field
//- @"0" ref/init Field
static struct s i = { .field = 0 };

//- @inner defines/binding Inner
struct sz { struct s inner; };
//- @inner ref/writes Inner
//- @"{ .field = 1 }" ref/init Inner
//- @field ref/writes Field
//- @"1" ref/init Field
static struct sz j = { .inner = { .field = 1 } };

//- @outer defines/binding Outer
struct ssz { struct sz outer; };
//- @outer ref/writes Outer
//- @"{ .inner = { .field = 2 } }" ref/init Outer
//- @inner ref/writes Inner
//- @"{ .field = 2 }" ref/init Inner
//- @field ref/writes Field
//- @"2" ref/init Field
static struct ssz k = { .outer = { .inner = { .field = 2 } } };


union U {
  //- @x defines/binding FieldX
  int x;
  //- @y defines/binding FieldY
  long y;
};

//- @"1" ref/init FieldY
//- !{ _ ref/init FieldX }
static union U u = {.y = 1};
