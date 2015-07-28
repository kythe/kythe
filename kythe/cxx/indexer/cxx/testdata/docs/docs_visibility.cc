// Checks that we don't raise visibility-related errors while parsing comments.

class C {
  /// \see f
  void f();
};
