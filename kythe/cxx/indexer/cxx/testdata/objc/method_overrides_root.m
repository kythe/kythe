// We emit overrides/root edges for objc.

//- @A defines/binding ADecl
@interface A
//- @foo defines/binding FooDecl
//- !{ FooDecl overrides/root _}
-(int)foo;
@end

//- @A defines/binding AImpl
@implementation A
//- @foo defines/binding FooImpl
//- !{ FooImpl overrides/root _}
-(int)foo {
  return 200;
}
@end

//- @B defines/binding BInterface
//- BInterface extends AImpl
@interface B : A
//- @foo defines/binding FooDecl2
//- FooDecl2 overrides FooDecl
//- FooDecl2 overrides/root FooDecl
-(int)foo;
@end

//- @B defines/binding BImpl
@implementation B
//- @foo defines/binding FooImpl2
//- FooImpl2 overrides FooDecl
//- FooImpl2 overrides/root FooDecl
-(int)foo {
  return 24;
}
@end

//- @C defines/binding CInterface
//- CInterface extends BImpl
@interface C : B
//- @foo defines/binding FooDecl3
//- FooDecl3 overrides FooDecl2
//- FooDecl3 overrides/root FooDecl
-(int)foo;
@end

//- @C defines/binding CImpl
@implementation C
//- @foo defines/binding FooImpl3
//- FooImpl3 overrides FooDecl2
//- FooImpl3 overrides/root FooDecl
-(int)foo {
  return 24;
}
@end
