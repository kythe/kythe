// We link from designated initializers back to the fields they initialize.

//- @field defines/binding Field
struct s { int field; };
//- @field ref Field
//- @"0" ref/init Field
static struct s i = { .field = 0 };

//- @inner defines/binding Inner
struct sz { struct s inner; };
//- @inner ref Inner
//- @"{ .field = 1 }" ref/init Inner
//- @field ref Field
//- @"1" ref/init Field
static struct sz j = { .inner = { .field = 1 } };

//- @outer defines/binding Outer
struct ssz { struct sz outer; };
//- @outer ref Outer
//- @"{ .inner = { .field = 2 } }" ref/init Outer
//- @inner ref Inner
//- @"{ .field = 2 }" ref/init Inner
//- @field ref Field
//- @"2" ref/init Field
static struct ssz k = { .outer = { .inner = { .field = 2 } } };
