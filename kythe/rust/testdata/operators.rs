#![feature(lang_items, no_core)]
#![no_core]

#[lang = "sized"]
trait Sized {}
#[lang = "legacy_receiver"]
trait LegacyReceiver {}
impl<T: ?Sized> LegacyReceiver for &T {}
#[lang = "clone"]
trait Clone {}
#[lang = "copy"]
trait Copy: Clone {}
#[lang = "eq"]
trait PartialEq<Rhs: ?Sized = Self> {
    fn eq(&self, other: &Rhs) -> bool;
    fn ne(&self, _other: &Rhs) -> bool {
        true
    }
}
#[lang = "index"]
trait Index<Idx: ?Sized> {
    type Output: ?Sized;
    fn index(&self, index: Idx) -> &Self::Output;
}

struct S(i32);
impl PartialEq for S {
    //- @eq defines/binding Eq
    fn eq(&self, _: &Self) -> bool {
        true
    }
}
impl Index<bool> for S {
    type Output = i32;
    //- @index defines/binding Index
    fn index(&self, _: bool) -> &i32 {
        &self.0
    }
}

fn test(a: S, b: S) {
    //- @"==" ref Eq
    a == b;
    //- @"[" ref Index
    &a[true];
}
