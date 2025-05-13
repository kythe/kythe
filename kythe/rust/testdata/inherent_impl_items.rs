#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @Struct defines/binding Struct
struct Struct;

//- ImplRange defines Impl
//- ImplRange.loc/start @^"impl"
//- Impl.node/kind record
//- Impl extends Struct
impl Struct {
    //- @Constant defines/binding Constant
    //- Constant childof Impl
    const Constant: isize = 42;

    //- @method defines/binding Method
    //- Method childof Impl
    //- @self defines/binding SelfParam
    //- SelfParam code SelfParamCode
    //- SelfParamCode.pre_text "&self: &Struct"
    //- @foo defines/binding FooParam
    //- Method param.0 SelfParam
    //- Method param.1 FooParam
    fn method(&self, foo: &u8) {}

    //- ImplRange.loc/end @$"}"
}

//- @Generic defines/binding Generic
struct Generic<T>(T);

//- _ defines ImplGeneric
//- ImplGeneric.node/kind record
//- ImplGeneric extends Generic
//- @T defines/binding T
//- T.node/kind tvar
//- ImplGeneric tparam.0 T
impl<T>
//- @T ref T
Generic<T> {
    //- @T ref T
    fn get(&self) -> &T { &self.0 }
}
