load("@bazel_skylib//lib:selects.bzl", "selects")

def with_or(mapping, no_match_error = ""):
    """Like selects.with_or, but allows duplicate conditions in the key tuples."""
    return select(with_or_dict(mapping), no_match_error = no_match_error)

def with_or_dict(mapping):
    """Like selects.with_or_dict, but allows duplicate condtions in tuples."""
    output_dict = {}
    for key, value in mapping.items():
        if type(key) == type(()):
            # Keep only unique values from the tuple.
            key = tuple({k: None for k in key if k})

        # Omit empty-keyed values.
        if key:
            output_dict[key] = value

    return selects.with_or_dict(output_dict)
