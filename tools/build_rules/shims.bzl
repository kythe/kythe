load("@io_bazel_rules_go//go:def.bzl", _go_test="go_test")

def go_test(name, library=None, **kwargs):
  """This macro wraps the go_test rule provided by the Bazel Go rules
  to silence a deprecation warning for use of the "library" attribute.
  It is otherwise equivalent in function to a go_test.
  """
  if not library:
    fail('Missing required "library" attribute')

  _go_test(
      name = name,
      embed = [library],
      **kwargs
  )
