// Anonymous namespaces that can be reached by headers are globally anonymous.
#pragma kythe_claim
#include "header.inc"
//- @namespace defines/binding CcNamespace
namespace { }
#include "header.h"

#example header.inc
#pragma kythe_claim
//- @namespace=HeaderDecl defines/binding HeaderNamespace
//- !{ HeaderDecl defines/binding CcNamespace }
namespace { }

#example header.h
#pragma kythe_claim
#include "header.inc"
