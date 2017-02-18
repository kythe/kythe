// The anonymous namespace is common among headers but distinct in TUs.
#pragma kythe_claim
#include "header.h"
//- @namespace ref CcNamespace
namespace { }
#include "footer.h"

#example header.h
#pragma kythe_claim
//- @namespace=HeaderDecl ref HNamespace
//- !{ HeaderDecl ref CcNamespace }
namespace { }

#example footer.h
#pragma kythe_claim
//- @namespace=FooterDecl ref HNamespace
//- !{ FooterDecl ref CcNamespace }
namespace { }
