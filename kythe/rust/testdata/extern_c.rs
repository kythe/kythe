mod foo {
    // TODO(b/335486573): emit csymbol names
    //- @yes defines/binding Yes = vname("[extern_c.rs]/~foo/=yes",_,_,_,_)
    extern "C" fn yes() {}
    extern "C" {
        //- @no defines/binding No = vname("[extern_c.rs]/~foo/=no",_,_,_,_)
        fn no();
    }
}
