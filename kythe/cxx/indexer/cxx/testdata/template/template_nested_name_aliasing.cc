// We emit references to nodes in nested names.
template <typename T>
//- @S defines/binding StructS
struct S {
  static T x;
  template <typename U>
  //- @V defines/binding _StructV
  struct V {
    static U y;
  };
};

//- @S ref _SInt
//- @V ref _VFloat
//- @int ref _Int
//- @float ref _Float
double z = S<int>::V<float>::y;

//- // SInt instantiates AppSInt  -- These edges are not emitted with aliasing.
//- // VFloat instantiates AppVFloat
//- _AppSInt param.0 StructS
//- _AppVFloat param.0 NominalV
//- NominalV.node/kind record
