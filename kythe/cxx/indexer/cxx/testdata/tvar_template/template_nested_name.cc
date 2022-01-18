// We emit references to nodes in nested names.
template <typename T>
//- @S defines/binding StructS
struct S {
  static T x;
  template <typename U>
//- @V defines/binding StructV
  struct V {
    static U y;
  };
};

//- @S ref SInt
//- @V ref VFloat
//- @int ref Int
//- @float ref Float
double z = S<int>::V<float>::y;

//- SInt instantiates AppSInt
//- VFloat instantiates AppVFloat
//- AppSInt param.0 StructS
//- AppVFloat param.0 NominalV
//- NominalV.node/kind record
