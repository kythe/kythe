// The anonymous namespace is common among headers but distinct in TUs.
#pragma kythe_claim
#include "header.h"
namespace Ns {
namespace {
//- @namespace ref CcNamespace
namespace { }
}
}
#include "footer.h"

#example header.h
#pragma kythe_claim
namespace Ns {
namespace {
//- @namespace=HeaderDecl ref HNamespace
//- !{ HeaderDecl ref CcNamespace }
namespace { }
}
}

#example footer.h
#pragma kythe_claim
namespace Ns {
namespace {
//- @namespace=FooterDecl ref HNamespace
//- !{ FooterDecl ref CcNamespace }
namespace { }
}
}
