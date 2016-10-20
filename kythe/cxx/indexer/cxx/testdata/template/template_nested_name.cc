// We emit references to nodes in nested names.
template <typename T>
//- @S defines/binding AbsS
struct S {
  static T x;
  template <typename U>
//- @V defines/binding AbsV
  struct V {
    static U y;
  };
};

//- @S ref SInt
//- @V ref VFloat
//- @int ref Int
//- @float ref Float
double z = S<int>::V<float>::y;

// Interestingly, we don't get the definition of V here:
//- SInt instantiates AppSInt
//- VFloat instantiates AppVFloat
//- AppSInt param.0 AbsS
//- AppVFloat param.0 NominalV
//- NominalV.node/kind tnominal
