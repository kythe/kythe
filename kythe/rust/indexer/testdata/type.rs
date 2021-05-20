// Verifies that the `type` keyword is handled properly by the indexer

//- @_TestType defines/binding TestType
//- TestType.node/kind talias
//- U32Type=vname("u32#builtin",_,_,_,"rust").node/kind tbuiltin
//- TestType aliases U32Type
type _TestType = u32;

fn main() {}
