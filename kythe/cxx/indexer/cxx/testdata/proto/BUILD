load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":cc_proto_verifier_test.bzl", "cc_proto_verifier_test")
load("//tools/build_rules/extra_aspects/cxx:rules.bzl", "cc_extract_kzip")
load("//tools/build_rules/verifier_test:cc_indexer_test.bzl", "cc_index")
load("//tools/build_rules/verifier_test:verifier_test.bzl", "verifier_test")

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

cc_proto_library(
    name = "testdata_cc_proto",
    deps = ["//kythe/testdata/indexers/proto:testdata_proto"],
)

cc_extract_kzip(
    name = "testdata_cc_proto_kzip",
    deps = [":testdata_cc_proto"],
)

genrule(
    name = "testdata_cc_proto_static_claim",
    srcs = [":testdata_cc_proto_kzip"],
    outs = ["testdata_cc_proto_static_claim.claim"],
    cmd = " ".join([
        "echo \"$<\" |",
        "$(location //kythe/cxx/tools:static_claim)",
        # Only claim protocol buffer headers and sources.
        # This won't work for things which aren't direct, immediate source files.
        "--include_files='.*kythe/testdata/indexers/proto/([^/]+.(pb|proto).(h|cc))$$'",
        "> \"$@\"",
    ]),
    tools = ["//kythe/cxx/tools:static_claim"],
)

cc_index(
    name = "exclude_proto_cc_entries",
    srcs = [":testdata_cc_proto_kzip"],
    opts = [
        # Try to reduce the graph size to make life easier for the verifier.
        "--test_claim",
        "--noindex_template_instantiations",
        "--experimental_drop_instantiation_independent_data",
        "--noemit_anchors_on_builtins",
        "--static_claim=$(execpath :testdata_cc_proto_static_claim)",
        "--claim_unknown=false",
        "--experimental_analysis_exclude_path_pattern=.*/[^/]+.pb.cc$",
    ],
    deps = [
        ":testdata_cc_proto_static_claim",
    ],
)

verifier_test(
    name = "exclude_proto_cc_test",
    size = "medium",
    srcs = ["exclude_proto_cc_test.txt"],
    opts = [
        "--use_fast_solver",
        "--nofile_vnames",
        "--ignore_dups",
        "--ignore_code_conflicts",
    ],
    deps = [":exclude_proto_cc_entries"],
)

bzl_library(
    name = "cc_proto_verifier_test_bzl",
    srcs = ["cc_proto_verifier_test.bzl"],
)
