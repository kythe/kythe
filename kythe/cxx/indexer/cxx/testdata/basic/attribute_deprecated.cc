// Check that we record information about [[deprecated]].

[[deprecated]]
//- @depf defines/binding DepF
//- DepF.tag/deprecated ""
void depf();

[[deprecated("reason")]]
//- @depfr defines/binding DepFR
//- DepFR.tag/deprecated "reason"
void depfr();

//- @C defines/binding ClassC
//- ClassC.tag/deprecated "reason2"
class [[deprecated("reason2")]] C;
