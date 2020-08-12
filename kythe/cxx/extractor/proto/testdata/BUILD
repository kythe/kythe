load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":proto_extractor_test.bzl", "extractor_golden_test")

extractor_golden_test(
    name = "relative_imports",
    srcs = ["relative_imports.proto"],
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
    deps = ["subdir/other.proto"],
)

# In this test, other.proto is both imported by relative_imports.proto and
# specified as a top-level proto to extract. Previously this caused an error
# because the extractor tried to import it twice and created duplicate symbol
# definitions.
extractor_golden_test(
    name = "duplicate_imports",
    srcs = [
        "relative_imports.proto",
        "subdir/other.proto",
    ],
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
)

# Similar to duplicate_imports test above, but with the two proto files passed
# to the extractor in reverse order.
extractor_golden_test(
    name = "duplicate_imports2",
    srcs = [
        "subdir/other.proto",
        # order matters for this test - don't sort these
        "relative_imports.proto",
    ],
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
)

# File paths in the output should all be relative to the testdata directory.
extractor_golden_test(
    name = "custom_root_directory",
    srcs = ["relative_imports.proto"],
    extra_env = {"KYTHE_ROOT_DIRECTORY": "kythe/cxx/extractor/proto/testdata"},
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
    deps = ["subdir/other.proto"],
)

extractor_golden_test(
    name = "simple",
    srcs = ["simple1.proto"],
)

extractor_golden_test(
    name = "multiple_source_files",
    srcs = [
        "simple1.proto",
        "simple2.proto",
    ],
)

extractor_golden_test(
    name = "custom_corpus",
    srcs = ["simple1.proto"],
    extra_env = {"KYTHE_CORPUS": "my_custom_corpus"},
)

extractor_golden_test(
    name = "custom_vname_config",
    srcs = [
        "simple1.proto",
        "simple2.proto",
    ],
    extra_env = {
        "KYTHE_VNAMES": "kythe/cxx/extractor/proto/testdata/custom_vname_config.json",
    },
    deps = ["custom_vname_config.json"],
)

sh_binary(
    name = "kzip_diff_test",
    srcs = ["kzip_diff_test.sh"],
    visibility = ["//kythe/cxx/extractor/textproto/testdata:__subpackages__"],
)

bzl_library(
    name = "proto_extractor_test_bzl",
    srcs = ["proto_extractor_test.bzl"],
)
