// The anonymous namespace is common among headers but distinct in TUs.
#pragma kythe_claim
#include "header.h"
//- @namespace defines/binding CcNamespace
namespace { }
#include "footer.h"

#example header.h
#pragma kythe_claim
//- @namespace=HeaderDecl defines/binding HNamespace
//- !{ HeaderDecl defines/binding CcNamespace }
namespace { }

#example footer.h
#pragma kythe_claim
//- @namespace=FooterDecl defines/binding HNamespace
//- !{ FooterDecl defines/binding CcNamespace }
namespace { }
