macro_rules! define {
    ($name:ident) => {
        struct $name;
    };
}
macro_rules! refr {
    ( $($name:tt)* ) => { $($name)* }
}

fn in_args() {
    let z: i32 = 0;
    //- @X defines/binding X
    define!(X);
    //- @X ref X
    let x: X;
    //- @X ref X
    let x: refr!(X);
    //- @X ref X
    let x: refr!(refr!(X));

    // @"struct Y" defines Y
    refr!(
        struct Y;
    );
}

macro_rules! define_y {
    () => {
        //- !{ @Y defines/binding _ }
        //- !{ @"struct Y" defines _ }
        //- !{ @"struct Y;" defines _ }
        struct Y;
    };
}

macro_rules! ref_y {
    () => {
        //- !{ @Y ref _ }
        Y
    };
}

macro_rules! structn {
    ($name:ident) => {
        struct $name;
    };
}

// Ranges within the macro body are resolved to zero-width anchors at the start
// of the macro call.
// TODO(b/405364596): This matches C++. Is this the ideal behavior?
fn in_body() {
    //- DefineYCallStart defines/binding Y
    //- DefineYCallStart.loc/start @^define_y
    //- DefineYCallStart.loc/end @^define_y
    define_y!();

    //- RefYCallStart ref Y
    //- RefYCallStart.loc/start @^ref_y
    //- RefYCallStart.loc/end @^ref_y
    let y: ref_y!();

    //- InnerRefCallStart ref Y
    //- InnerRefCallStart.loc/start @^ref_y
    //- InnerRefCallStart.loc/end @^ref_y
    let y: refr!(ref_y!());

    //- DefineCallStart defines Z
    //- DefineCallStart.loc/start @^define
    //- DefineCallStart.loc/end @^define
    define!(Z);
    //- @Z ref Z
    let z: Z;
}
