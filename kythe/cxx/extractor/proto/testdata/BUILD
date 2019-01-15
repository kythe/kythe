load(":proto_extractor_test.bzl", "proto_extraction_golden_test")

proto_extraction_golden_test(
    name = "relative_imports",
    extra_files = ["subdir/other.proto"],
    protoc_args = [
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
    protos = ["relative_imports.proto"],
)

# In this test, other.proto is both imported by relative_imports.proto and
# specified as a top-level proto to extract. Previously this caused an error
# because the extractor tried to import it twice and created duplicate symbol
# definitions.
proto_extraction_golden_test(
    name = "duplicate_imports",
    protoc_args = [
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
    protos = [
        "relative_imports.proto",
        "subdir/other.proto",
    ],
)

# Similar to duplicate_imports test above, but with the two proto files passed
# to the extractor in reverse order.
proto_extraction_golden_test(
    name = "duplicate_imports2",
    protoc_args = [
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
    protos = [
        "subdir/other.proto",
        # order matters for this test - don't sort these
        "relative_imports.proto",
    ],
)

# File paths in the output should all be relative to the testdata directory.
proto_extraction_golden_test(
    name = "custom_root_directory",
    extra_env = {"KYTHE_ROOT_DIRECTORY": "kythe/cxx/extractor/proto/testdata"},
    extra_files = ["subdir/other.proto"],
    protoc_args = [
        "--proto_path",
        "kythe/cxx/extractor/proto/testdata/subdir",
    ],
    protos = ["relative_imports.proto"],
)

proto_extraction_golden_test(
    name = "simple",
    protos = ["simple1.proto"],
)

proto_extraction_golden_test(
    name = "multiple_source_files",
    protos = [
        "simple1.proto",
        "simple2.proto",
    ],
)

proto_extraction_golden_test(
    name = "custom_corpus",
    extra_env = {"KYTHE_CORPUS": "my_custom_corpus"},
    protos = ["simple1.proto"],
)

proto_extraction_golden_test(
    name = "custom_vname_config",
    extra_env = {
        "KYTHE_VNAMES": "kythe/cxx/extractor/proto/testdata/custom_vname_config.json",
    },
    extra_files = ["custom_vname_config.json"],
    protos = [
        "simple1.proto",
        "simple2.proto",
    ],
)
