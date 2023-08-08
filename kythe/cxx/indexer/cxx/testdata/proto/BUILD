load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":cc_proto_verifier_test.bzl", "cc_proto_verifier_test")

package(features = ["layering_check"])

cc_proto_verifier_test(
    name = "proto_test",
    size = "medium",
    srcs = ["proto.cc"],
    proto_libs = ["//kythe/testdata/indexers/proto:testdata_proto"],
)

cc_proto_verifier_test(
    name = "proto_semantic_ref_test",
    size = "medium",
    srcs = ["proto_semantic_ref.cc"],
    proto_libs = ["//kythe/testdata/indexers/proto:testdata_proto"],
    verifier_opts = [
        "--ignore_dups",
        "--convert_marked_source",
        "--check_for_singletons=false",
    ],
)

cc_proto_verifier_test(
    name = "proto_semantic_ref_aw_test",
    size = "medium",
    srcs = ["proto_semantic_ref_aw.cc"],
    experimental_set_aliases_as_writes = True,
    proto_libs = ["//kythe/testdata/indexers/proto:testdata_proto"],
    verifier_opts = [
        "--ignore_dups",
        "--convert_marked_source",
        "--check_for_singletons=false",
    ],
)

cc_proto_verifier_test(
    name = "proto_semantic_test",
    size = "medium",
    srcs = ["proto_semantic.cc"],
    proto_libs = ["//kythe/testdata/indexers/proto:testdata_proto"],
    verifier_opts = [
        "--ignore_dups",
        "--convert_marked_source",
        "--check_for_singletons=false",
    ],
)

cc_proto_verifier_test(
    name = "proto2_test",
    size = "medium",
    srcs = ["proto2.cc"],
    proto_libs = ["//kythe/testdata/indexers/proto:testdata2_proto"],
)

cc_proto_verifier_test(
    name = "proto3_test",
    size = "medium",
    srcs = ["proto3.cc"],
    proto_libs = [
        "//kythe/testdata/indexers/proto:testdata3_proto",
        "//kythe/testdata/indexers/proto:testdata3a_proto",
        "//kythe/testdata/indexers/proto:testdata3b_proto",
    ],
)

cc_proto_verifier_test(
    name = "proto4_test",
    size = "medium",
    srcs = ["proto4.cc"],
    proto_libs = [
        "//kythe/testdata/indexers/proto:testdata4a_proto",
        "//kythe/testdata/indexers/proto:testdata4b_proto",
        "//kythe/testdata/indexers/proto:testdata4c_proto",
    ],
)

cc_proto_verifier_test(
    name = "proto5_test",
    size = "medium",
    srcs = ["proto5.cc"],
    proto_libs = [
        "//kythe/testdata/indexers/proto:testdata5_proto",
    ],
)

cc_proto_verifier_test(
    name = "enum_test",
    size = "medium",
    srcs = ["enum.cc"],
    proto_libs = [
        "//kythe/testdata/indexers/proto:enum_proto",
    ],
)

bzl_library(
    name = "cc_proto_verifier_test_bzl",
    srcs = ["cc_proto_verifier_test.bzl"],
)
