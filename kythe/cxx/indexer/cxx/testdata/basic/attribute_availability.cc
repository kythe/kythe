// Check that we record information about [[clang::availability]].

[[clang::availability(macos,deprecated=10.6)]]
//- @depf defines/binding DepF
//- DepF.tag/deprecated "10.6"
void depf();

[[clang::availability(macos,deprecated=10.7)]]
//- @depfr defines/binding DepFR
//- DepFR.tag/deprecated "10.7"
void depfr();

//- @C defines/binding ClassC
//- ClassC.tag/deprecated "10.8"
class [[clang::availability(macos,deprecated=10.8)]] C;
