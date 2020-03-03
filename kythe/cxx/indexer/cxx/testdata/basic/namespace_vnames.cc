// We assign the correct vnames to namespaces.
#pragma kythe_claim
#include "acorpus_aroot_apath.h"
// Anonymous namespaces retain path, root, and corpus.
//- @namespace ref vname(_,"bundle","","test.cc","c++")
namespace { }

// Named namespaces drop path and root and use the default corpus.
//- @ns ref vname(_,"","","","c++")
namespace ns { }
#example acorpus_aroot_apath.h
#pragma kythe_claim

// Anonymous namespaces in headers drop path and root, but use the file corpus.
//- @namespace ref vname(_,"acorpus","","","c++")
namespace { }

// Named namespaces behave the same way in headers and sources.
//- @ns ref vname(_,"","","","c++")
namespace ns { }
