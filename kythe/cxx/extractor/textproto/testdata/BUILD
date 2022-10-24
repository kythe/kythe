load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":textproto_extractor_test.bzl", "textproto_extractor_golden_test")

package(features = ["layering_check"])

textproto_extractor_golden_test(
    name = "simple",
    srcs = ["simple.pbtxt"],
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata",
    ],
    deps = ["example.proto"],
)

# This test specifies example_with_deps.proto as the main proto file, which
# should cause its dependency (deps.proto) to be extracted as well.
textproto_extractor_golden_test(
    name = "deps",
    srcs = ["deps.pbtxt"],
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata",
    ],
    deps = [
        "dep.proto",
        "example_with_deps.proto",
    ],
)

# Test that specifying proto message and file via flags works. There are no
# schema comments in the textproto.
textproto_extractor_golden_test(
    name = "without_schema",
    srcs = ["without_schema.pbtxt"],
    opts = [
        "--proto_files",
        "example.proto",
        "--proto_message",
        "textproto_test.MyMessage",
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata",
    ],
    deps = [
        "example.proto",
    ],
)

# Test that the "# proto-import:" schema comment works.
textproto_extractor_golden_test(
    name = "extra_imports",
    srcs = ["extra_imports.pbtxt"],
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata",
    ],
    deps = [
        "dep.proto",
        "example.proto",
    ],
)

# Test that --proto_files flag handles multiple protos correctly.
textproto_extractor_golden_test(
    name = "multiple_proto_files",
    srcs = ["without_schema.pbtxt"],
    opts = [
        "--proto_message=textproto_test.MyMessage",
        "--proto_files=example.proto,dep.proto",
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata",
    ],
    deps = [
        "dep.proto",
        "example.proto",
    ],
)

# Test that KYTHE_CORPUS environment variable works.
textproto_extractor_golden_test(
    name = "custom_corpus",
    srcs = ["simple.pbtxt"],
    extra_env = {"KYTHE_CORPUS": "custom_corpus"},
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata",
    ],
    deps = ["example.proto"],
)

# Test that KYTHE_ROOT_DIRECTORY environment variable works. File paths in the
# output should be relative to KYTHE_ROOT_DIRECTORY, not the extractor's working
# directory.
textproto_extractor_golden_test(
    name = "subdir",
    srcs = ["subdir/subdir.pbtxt"],
    extra_env = {"KYTHE_ROOT_DIRECTORY": "kythe/cxx/extractor/textproto/testdata/subdir"},
    opts = [
        "--",
        "--proto_path",
        "kythe/cxx/extractor/textproto/testdata/subdir",
    ],
    deps = ["subdir/subdir.proto"],
)

bzl_library(
    name = "textproto_extractor_test_bzl",
    srcs = ["textproto_extractor_test.bzl"],
)
