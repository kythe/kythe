// We link from designated initializers back to the fields they initialize.

//- @field defines/binding Field
struct s { int field; };
//- @field ref/init Field
static struct s i = { .field = 0 };

//- @inner defines/binding Inner
struct sz { struct s inner; };
//- @inner ref/init Inner
//- @field ref/init Field
static struct sz j = { .inner = { .field = 1 } };

//- @outer defines/binding Outer
struct ssz { struct sz outer; };
//- @outer ref/init Outer
//- @inner ref/init Inner
//- @field ref/init Field
static struct ssz k = { .outer = { .inner = { .field = 2 } } };
