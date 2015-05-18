// Defines flags of every type supported by gflags.
#include "gflags.h"
//- @boolflag defines BoolFlag
//- BoolFlag.node/kind google/gflag
DEFINE_bool(boolflag, true, "a");
//- @int32flag defines Int32Flag
//- Int32Flag.node/kind google/gflag
DEFINE_int32(int32flag, 1, "b");
//- @int64flag defines Int64Flag
//- Int64Flag.node/kind google/gflag
DEFINE_int64(int64flag, 2, "c");
//- @uint64flag defines UInt64Flag
//- UInt64Flag.node/kind google/gflag
DEFINE_uint64(uint64flag, 3, "d");
//- @doubleflag defines DoubleFlag
//- DoubleFlag.node/kind google/gflag
DEFINE_double(doubleflag, 4.0, "e");
//- @stringflag defines StringFlag
//- StringFlag.node/kind google/gflag
DEFINE_string(stringflag, "five", "f");
//- @FLAGS_boolflag ref BoolFlag
auto bref = FLAGS_boolflag;
//- @FLAGS_int32flag ref Int32Flag
auto iref = FLAGS_int32flag;
//- @FLAGS_int64flag ref Int64Flag
auto lref = FLAGS_int64flag;
//- @FLAGS_uint64flag ref UInt64Flag
auto uref = FLAGS_uint64flag;
//- @FLAGS_doubleflag ref DoubleFlag
auto dref = FLAGS_doubleflag;
//- @FLAGS_stringflag ref StringFlag
auto sref = FLAGS_stringflag;
