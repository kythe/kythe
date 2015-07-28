// We don't crash on linkage specs.

extern "C" {
  void foo() {
    /// \see foo
    int x;
  }
}
