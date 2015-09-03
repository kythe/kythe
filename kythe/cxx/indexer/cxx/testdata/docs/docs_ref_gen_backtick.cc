// Checks that we generate basic references from Markdown backticks.
///- @bar ref/doc FnBar
/// `bar`
///- @bar defines/binding FnBar
int bar() {
  return 0;
}
//- goal_prefix should be ///-
