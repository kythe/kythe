trait Bound {}

// Test type parameters:
//- @"T: Bound" defines FooT
//- @T defines/binding FooT
//- FooT code FooTCode
//- FooTCode.pre_text "T: Bound + ?Sized"
//- @U defines/binding FooU
fn foo<T: Bound, U>(
    //- @T ref FooT
    _t: T,
    //- @U ref FooU
) -> U
where
    //- @U ref FooU
    U: Copy + PartialEq,
{
    unimplemented!()
}

// Test const generic parameters:
//- @T defines/binding BarT
//- BarT code BarTCode
//- BarTCode.pre_text "const T: usize"
//- @"const U: usize = 10" defines BarU
//- @U defines/binding BarU
struct Bar<const T: usize, const U: usize = 10> {
    //- @T ref BarT
    t: [i8; T],
    //- @U ref BarU
    u: [i16; U],
}

// Test lifetime parameters:
fn baz<
    //- @"'a" defines/binding BazA
    'a,
    //- @"'b" defines/binding BazB
    'b,
    //- @"'c: 'a + 'b" defines BazC
    //- @"'c" defines/binding BazC
    //- @"'a" ref BazA
    //- @"'b" ref BazB
    //- BazC code BazCCode
    //- BazCCode.pre_text "'c" // TODO(b/407042236): Include lifetime bounds in hover card text.
    'c: 'a + 'b,
>(
    //- @"'a" ref BazA
    _x: &'a u8,
    //- @"'b" ref BazB
    _y: &'b u8,
    //- @"'c" ref BazC
) -> &'c mut u8
where
    //- @"'b" ref BazB
    //- @"'a" ref BazA
    'b: 'a,
{
    unimplemented!()
}
