package(features = ["layering_check"])

cc_binary(
    name = "textproto_indexer",
    srcs = ["textproto_indexer_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":analyzer",
        ":plugin_registry",
        "//kythe/cxx/common:init",
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
        ":plugin",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/common:status_or",
        "//kythe/cxx/common:utf8_line_index",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/extractor/textproto:textproto_schema",
        "//kythe/cxx/indexer/proto:offset_util",
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

cc_library(
    name = "plugin",
    hdrs = ["plugin.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/cxx/common/indexing:output",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "plugin_registry",
    srcs = ["plugin_registry.cc"],
    hdrs = ["plugin_registry.h"],
    deps = [
        ":plugin",
        "//kythe/cxx/indexer/textproto/plugins/example:plugin",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)
