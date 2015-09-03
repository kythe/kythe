// Declares flags of every type supported by gflags.
#include "gflags.h"
//- @boolflag defines/binding BoolFlag
//- BoolFlag.node/kind google/gflag
DECLARE_bool(boolflag);
//- @int32flag defines/binding Int32Flag
//- Int32Flag.node/kind google/gflag
DECLARE_int32(int32flag);
//- @int64flag defines/binding Int64Flag
//- Int64Flag.node/kind google/gflag
DECLARE_int64(int64flag);
//- @uint64flag defines/binding UInt64Flag
//- UInt64Flag.node/kind google/gflag
DECLARE_uint64(uint64flag);
//- @doubleflag defines/binding DoubleFlag
//- DoubleFlag.node/kind google/gflag
DECLARE_double(doubleflag);
//- @stringflag defines/binding StringFlag
//- StringFlag.node/kind google/gflag
DECLARE_string(stringflag);
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
