load("//tools/cpp:cc_release_binary.bzl", "cc_release_binary")

package(features = ["layering_check"])

cc_release_binary(
    name = "textproto_indexer",
    srcs = ["textproto_indexer_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":analyzer",
        "//kythe/cxx/common:kzip_reader",
        "//kythe/cxx/common/indexing:caching_output",
        "//kythe/cxx/common/indexing:output",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/status",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "analyzer",
    srcs = ["analyzer.cc"],
    hdrs = ["analyzer.h"],
    deps = [
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/common:status_or",
        "//kythe/cxx/common:utf8_line_index",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/extractor/textproto:textproto_schema",
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/cxx/indexer/proto:source_tree",
        "//kythe/cxx/indexer/proto:vname_util",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)
