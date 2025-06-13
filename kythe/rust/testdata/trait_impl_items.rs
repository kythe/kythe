#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @Struct defines/binding Struct
struct Struct;

//- @Trait defines/binding Trait
trait Trait {
    //- @Type defines/binding TraitType
    type Type;

    //- @CONST defines/binding TraitConst
    const CONST: u8;

    //- @method defines/binding TraitMethod
    fn method(&self);
}

//- ImplRange defines Impl
//- Impl.node/kind record
//- Impl extends Struct
//- Impl extends Trait
//- ImplRange.loc/start @^"impl"
impl Trait for Struct {

    //- @Type defines/binding ImplType
    //- ImplType overrides TraitType
    type Type = u8;

    //- @CONST defines/binding ImplConst
    //- ImplConst overrides TraitConst
    const CONST: u8 = 45;

    //- @method defines/binding ImplMethod
    //- ImplMethod overrides TraitMethod
    fn method(&self) {}

    //- ImplBarRange.loc/end @$"}"
}

//- @GenericStruct defines/binding GenericStruct
struct GenericStruct<T>(T);

//- @GenericTrait defines/binding GenericTrait
trait GenericTrait<'a> {
    //- @baz defines/binding TraitBaz
    fn baz(x: &'a u8);
}

//- _ defines GenericImpl
//- GenericImpl extends GenericStruct
//- GenericImpl extends GenericTrait
//- @"'a" defines/binding LifetimeA
//- @U defines/binding U
//- GenericImpl tparam.0 LifetimeA
//- GenericImpl tparam.1 U
impl<'a, U>
//- @"'a" ref LifetimeA
//- @U ref U
GenericTrait<'a> for GenericStruct<U> {
    //- @baz defines/binding ImplBaz
    //- ImplBaz overrides TraitBaz
    fn baz(x: &'a u8) {}
}
