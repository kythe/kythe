//- @Trait defines/binding Trait
//- Trait.node/kind interface
//- TraitRange defines Trait
//- TraitRange.loc/start @^"trait"
//- Trait code TraitCode
//- TraitCode.pre_text "trait Trait\nwhere\n    Self: Default,"
trait Trait: Default {
    //- @function defines/binding Function
    //- Function.node/kind function
    //- Function childof Trait
    fn function(x: u16) -> String;

    //- @default_function defines/binding DefaultFunction
    //- DefaultFunction.node/kind function
    //- DefaultFunction childof Trait
    fn default_function() {
        // Has a default implementation.
    }

    //- @CONST defines/binding Const
    //- Const.node/kind constant
    //- Const childof Trait
    const CONST: &str;

    //- @DEFAULT_CONST defines/binding DefaultConst
    //- DefaultConst.node/kind constant
    //- DefaultConst childof Trait
    const DEFAULT_CONST: &str = "has a default value";

    //- @Alias defines/binding Alias
    //- Alias.node/kind talias
    //- Alias childof Trait
    type Alias;

    //- TraitRange.loc/end @$"}"
}

//- @Generic defines/binding Generic
//- @"'a" defines/binding LifetimeA
//- @T defines/binding T
//- Generic tparam.0 LifetimeA
//- Generic tparam.1 Self
//- Generic tparam.2 T
//- Generic code GenericCode
//- GenericCode.pre_text "trait Generic<'a, T>"
trait Generic<'a, T> {
    //- @foo defines/binding Foo
    //- Foo.node/kind function
    //- Foo childof Generic
    //- @"'a" ref LifetimeA
    //- @T ref T
    fn foo(x: &'a T);

    //- @method defines/binding Method
    //- Method.node/kind function
    //- Method childof Generic
    //- @self defines/binding SelfParam
    //- @x defines/binding X
    //- Method param.0 SelfParam
    //- Method param.1 X
    fn method(&self, x: u8);
}
