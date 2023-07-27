# Minimum supported Bazel version.  Should match .bazelminversion file.
MIN_VERSION = "6.3.0"

# Maximum supported Bazel version.  Should match .bazelversion file.
MAX_VERSION = "6.3.0"

def _tuplicate(value, delim):
    rv = ()
    for field in value.split(delim):
        if field.isdigit():
            rv += (int(field),)
        else:
            rv += (field,)
    return rv

def _parse_version(version):
    if not version:
        return ()

    # Remove any commit tail.
    version = version.split(" ", 1)[0]

    # Split into (release, date) parts.
    parts = version.split("-", 1)
    return _tuplicate(parts[0], ".")

def _bound_size(tup, size, padding = 0):
    if len(tup) >= size:
        return tup[:size]
    ret = tup

    for i in range(size - len(tup)):
        ret += (padding,)
    return ret

def check_version(min_required, max_supported):
    found = native.bazel_version
    if not found:
        print("\nDevelopment version of bazel detected.\nDisabling version check.\nExpect the unexpected.")
        return
    elif "rc" in found:
        print("\nRelease candidate version of bazel detected: %s.\nDisabling version check.\nGood luck!" % (found,))
        return
    found_version = _parse_version(found)
    min = _parse_version(min_required)
    if min > _bound_size(found_version, len(min)):
        fail("You need to update bazel. Required version {} of bazel, found {}".format(min_required, found))
    max = _parse_version(max_supported)
    if max < _bound_size(found_version, len(max)):
        fail("Your bazel is too new. Maximum supported version {} of bazel, found {}".format(max_supported, found))

def prohibit_version(version, reason):
    found = _parse_version(native.bazel_version)
    if found == _parse_version(version):
        fail("\n".join([
            "You're using a prohibited version of Bazel ({}).".format(native.bazel_version),
            "Please upgrade or downgrade to a supported release.",
            "The version of Bazel you're using is incompatible with Kythe because:",
            reason,
        ]))
