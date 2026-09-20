#[rustfmt::skip] const _ : () = { struct S; let _ : S; mod a { struct T; const _ : T = T; } };
//       1         2         3         4         5         6         7         8         9
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

// This tests vnames of items that need positional information to disambiguate.
// Sorry about the strange formatting - the tests are very fragile.
// We can't use @anchors, because those can only point forwards in the file, not backwards.
// TODO(b/404595611): remove extra `_` variables that only exist to ensure everything is referenced.

// The top-level const item. It has positional information because it is unnamed.
//- _ defines vname("[scope_vnames.rs]/=definition[0:94]",_,_,_,_)

// The struct S. It has positional information because it's in an anonymous block.
//- _ defines vname("[scope_vnames.rs]/~S[34:43]",_,_,_,_)

// The struct T. It has no extra positional info because it's unique within `mod a`.
//- _ defines vname("[scope_vnames.rs]/~a[55:91]/~T",_,_,_,_)
