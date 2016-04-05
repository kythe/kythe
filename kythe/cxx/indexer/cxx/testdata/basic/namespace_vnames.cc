// We assign the correct vnames to namespaces.
#pragma kythe_claim
#include "acorpus_aroot_apath.h"
// Anonymous namespaces retain path, root, and corpus.
//- @namespace defines/binding vname(_,"bundle","","test.cc","c++")
namespace { }

// Named namespaces drop path and root but keep corpus.
//- @ns defines/binding vname(_,"bundle","","","c++")
namespace ns { }
#example acorpus_aroot_apath.h
#pragma kythe_claim

// Anonymous namespaces in headers act like named namespaces in headers.
//- @namespace defines/binding vname(_,"acorpus","","","c++")
namespace { }

// Named namespaces in headers drop path and root and adopt the corpus of
// the surrounding include.
//- @ns defines/binding vname(_,"acorpus","","","c++")
namespace ns { }
