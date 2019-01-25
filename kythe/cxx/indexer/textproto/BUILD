cc_binary(
    name = "textproto_indexer",
    srcs = ["textproto_indexer_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":analyzer",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:kzip_reader",
        "@io_kythe//kythe/cxx/common/indexing:caching_output",
        "@io_kythe//kythe/cxx/common/indexing:output",
        "@io_kythe//kythe/proto:analysis_cc_proto",
    ],
)

cc_library(
    name = "analyzer",
    srcs = ["analyzer.cc"],
    hdrs = ["analyzer.h"],
    deps = [
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/cxx/indexer/proto:source_tree",
        "//kythe/cxx/indexer/proto:vname_util",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:path_utils",
        "@io_kythe//kythe/cxx/common:utf8_line_index",
        "@io_kythe//kythe/cxx/common/indexing:output",
        "@io_kythe//kythe/proto:analysis_cc_proto",
    ],
)
