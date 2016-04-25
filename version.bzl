def _tuplicate(value, delim):
  rv = ()
  for field in value.split(delim):
    if field.isdigit():
      rv += (int(field),)
    else:
      rv += (field,)
  return rv

def _parse_version(version):
  # Remove any commit tail.
  version = version.split(" ", 1)[0]
  # Split into (release, date) parts.
  parts = version.split('-', 1)
  if len(parts) == 2:
    return (_tuplicate(parts[0], '.'), _tuplicate(parts[1], '-'))
  else:
    return (_tuplicate(parts[0], '.'), ())

def check_version(required):
  found = native.bazel_version
  if _parse_version(required) > _parse_version(found):
    fail("Required version {} of bazel, found {}".format(required, found))
