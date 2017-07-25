// Anonymous namespaces that can be reached by headers are globally anonymous.
#pragma kythe_claim
#include "header.inc"
//- @namespace ref CcNamespace
namespace { }
#include "header.h"

#example header.inc
#pragma kythe_claim
//- @namespace=HeaderDecl ref _HeaderNamespace
//- !{ HeaderDecl ref CcNamespace }
namespace { }

#example header.h
#pragma kythe_claim
#include "header.inc"
