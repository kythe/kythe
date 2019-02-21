load("//tools/build_rules/verifier_test:cc_indexer_test.bzl", "cc_kythe_proto_library", "cc_index", "cc_extract_kzip")
load("//tools/build_rules/verifier_test:verifier_test.bzl", "verifier_test", "index_compilation")
load(
    "@io_kythe_lang_proto//kythe/cxx/indexer/proto/testdata:proto_verifier_test.bzl",
    "proto_extract_kzip",
)

proto_extract_kzip(
    name = "testdata_proto_kzip",
    srcs = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata.proto",
    ],
)

index_compilation(
    name = "testdata_proto_entries",
    indexer = "@io_kythe_lang_proto//kythe/cxx/indexer/proto:indexer",
    opts = ["--index_file"],
    deps = [":testdata_proto_kzip"],
)

cc_kythe_proto_library(
    name = "testdata_cc_proto",
    deps = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata_proto"],
)

cc_extract_kzip(
    name = "proto_cc_kzip",
    srcs = [
        "proto.cc",
    ],
    deps = [":testdata_cc_proto"],
)

cc_index(
    name = "proto_cc_entries",
    srcs = [":proto_cc_kzip"],
    opts = [
        # Avoid emitting some nodes with conflicting facts.
        "--experimental_index_lite",
        # Try to reduce the graph size to make life easier for the verifier.
        "--test_claim",
        "--noindex_template_instantiations",
        "--experimental_drop_instantiation_independent_data",
        "--noemit_anchors_on_builtins",
    ],
)

verifier_test(
    name = "proto_test",
    srcs = [
        "proto.cc",
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata.proto",
    ],
    opts = [
        "--ignore_dups",
        # Else the verifier chokes on the inconsistent marked source from the protobuf headers.
        "--convert_marked_source",
    ],
    deps = [
        ":proto_cc_entries",
        ":testdata_proto_entries",
    ],
)
