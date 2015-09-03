// Checks that we generate basic references from Doxygen commands.
///- @bar ref/doc FnBar
/// \c bar
///- @bar ref/doc FnBar
/// \unknown bar
///- @bar defines/binding FnBar
int bar() {
  return 0;
}
//- goal_prefix should be ///-
