// Tests the behavior of alias definitions combined with cvr-qualifiers.
// (Note that we don't currently record canonical types.)
//- @const_int defines ConstInt
using const_int = const int;
//- @volatile_int defines VolatileInt
using volatile_int = volatile int;
//- @cv_int_1 defines CvInt1
using cv_int_1 = const volatile_int;
//- @cv_int_2 defines CvInt2
using cv_int_2 = volatile const_int;
//- CvInt1.node/kind talias
//- CvInt2.node/kind talias
//- VolatileInt.node/kind talias
//- ConstInt.node/kind talias
//- ConstInt aliases ConstIntT
//- VolatileInt aliases VolatileIntT
//- CvInt1 aliases CvInt1T
//- CvInt2 aliases CvInt2T
//- CvInt1T.node/kind tapp
//- CvInt2T.node/kind tapp
//- CvInt1T param.1 VolatileInt
//- CvInt2T param.1 ConstInt
