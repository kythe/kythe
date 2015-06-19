// Checks that we correctly record instantiates edges for total specs.
//- @s_equals_float defines PrimaryT
template <typename S> bool s_equals_float = false;
//- @s_equals_float defines TotalT
template <> bool s_equals_float<float> = true;
//- @s_equals_float ref TotalT
//- TotalT instantiates PrimaryTFloat
//- TotalT specializes PrimaryTFloat
//- PrimaryTFloat param.0 PrimaryT
bool is_true = s_equals_float<float>;
