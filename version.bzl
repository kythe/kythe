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

def check_version(min_required, max_supported):
  found = native.bazel_version
  found_version = _parse_version(found)[0]
  if _parse_version(min_required)[0] > found_version:
    fail("You need to update bazel. Required version {} of bazel, found {}".format(min_required, found))
  if _parse_version(max_supported)[0] < found_version:
    fail("Your bazel is too new. Maximum supported version {} of bazel, found {}".format(max_supported, found))
