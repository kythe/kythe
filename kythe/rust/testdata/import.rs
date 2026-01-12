import::import! {
    //- @"\"//kythe/rust/testdata:dep\"" ref ProcMacroDep
    "//kythe/rust/testdata:dep" as mydep;
}

//- @Y defines/binding Y
//- @X ref X
//- @mydep ref Dep = vname("[dep.rs]","google","","","rust")
const Y: u32 = mydep::X;
