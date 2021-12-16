
template<typename T>
void fn() {
  //- @value_type ref DependentName
  typename T::value_type a;
  //- @value_type ref DependentName
  typename T::value_type b;
}
