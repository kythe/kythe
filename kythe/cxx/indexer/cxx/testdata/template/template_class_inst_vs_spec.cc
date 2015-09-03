// Checks that instantiates/specializes are applied to class templates.
//- @C defines/binding PrimaryT
template<typename T, typename S> class C {};
//- @C defines/binding PartialT
template<typename S> class C<int, S> {};
//- @C defines/binding TotalT
template<> class C<float, double> {};

// Specializes and instantiates primary
//- @Primary defines/binding APrimary
//- APrimary typed PrimaryLongShort
//- PrimaryLongShort param.0 PrimaryT
//- PrimaryImp specializes PrimaryLongShort
//- PrimaryImp instantiates PrimaryLongShort
C<long, short> Primary;

// Specializes primary and instantiates partial
// NB: the type is still PrimaryT<int, short>.
//- @PartialSpec defines/binding APartialSpec
//- APartialSpec typed PrimaryIntShort
//- ImpPartialSpec specializes PrimaryIntShort
//- PrimaryIntShort param.0 PrimaryT
//- ImpPartialSpec instantiates PartialIntShort
//- PartialIntShort param.0 PartialT
//- PartialIntShort param.1 TShort
//- @short ref TShort
C<int, short> PartialSpec;

// Specializes primary and instantiates primary
//- @TotalSpec defines/binding ATotalSpec
//- ATotalSpec typed PrimaryFloatDouble
//- TotalT instantiates PrimaryFloatDouble
//- TotalT specializes PrimaryFloatDouble
C<float, double> TotalSpec;
