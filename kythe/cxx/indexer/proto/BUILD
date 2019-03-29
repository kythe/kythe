package(
    features = [
        "layering_check",
        "parse_headers",
    ],
)

cc_library(
    name = "proto_graph_builder",
    srcs = ["proto_graph_builder.cc"],
    hdrs = ["proto_graph_builder.h"],
    deps = [
        ":comments",
        ":vname_util",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:kythe_uri",
        "@io_kythe//kythe/cxx/common:lib",
        "@io_kythe//kythe/cxx/common:vname_ordering",
        "@io_kythe//kythe/cxx/common/indexing:output",
        "@io_kythe//kythe/proto:common_cc_proto",
        "@io_kythe//kythe/proto:storage_cc_proto",
        "@io_kythe//kythe/proto:xref_cc_proto",
    ],
)

cc_library(
    name = "proto_analyzer",
    srcs = [
        "file_descriptor_walker.cc",
        "indexer_frontend.cc",
        "marked_source.cc",
        "proto_analyzer.cc",
    ],
    hdrs = [
        "file_descriptor_walker.h",
        "indexer_frontend.h",
        "marked_source.h",
        "proto_analyzer.h",
    ],
    copts = [
        "-Wno-unused-variable",
    ],
    deps = [
        ":proto_graph_builder",
        ":search_path",
        ":source_tree",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:node_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
        "@io_kythe//kythe/cxx/common:kythe_uri",
        "@io_kythe//kythe/cxx/common:lib",
        "@io_kythe//kythe/cxx/common:path_utils",
        "@io_kythe//kythe/cxx/common:status_or",
        "@io_kythe//kythe/cxx/common:utf8_line_index",
        "@io_kythe//kythe/cxx/common/indexing:output",
        "@io_kythe//kythe/proto:analysis_cc_proto",
        "@io_kythe//kythe/proto:common_cc_proto",
        "@io_kythe//kythe/proto:storage_cc_proto",
        "@io_kythe//kythe/proto:xref_cc_proto",
    ],
)

cc_library(
    name = "vname_util",
    hdrs = ["vname_util.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:lib",
    ],
)

cc_library(
    name = "source_tree",
    srcs = ["source_tree.cc"],
    hdrs = ["source_tree.h"],
    visibility = [
        "//kythe/cxx/indexer/textproto:__subpackages__",
    ],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:path_utils",
    ],
)

cc_library(
    name = "search_path",
    srcs = ["search_path.cc"],
    hdrs = ["search_path.h"],
    visibility = [
        "//kythe/cxx/extractor/proto:__subpackages__",
        "//kythe/cxx/extractor/textproto:__subpackages__",
        "//kythe/cxx/indexer/textproto:__subpackages__",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:path_utils",
    ],
)

cc_binary(
    name = "indexer",
    visibility = ["//visibility:public"],
    deps = [":cmdlib"],
)

cc_library(
    name = "cmdlib",
    srcs = ["kythe_indexer_main.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":proto_analyzer",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:json_proto",
        "@io_kythe//kythe/cxx/common:kzip_reader",
        "@io_kythe//kythe/cxx/common:lib",
        "@io_kythe//kythe/cxx/common:path_utils",
        "@io_kythe//kythe/cxx/common/indexing:caching_output",
        "@io_kythe//kythe/cxx/common/indexing:output",
        "@io_kythe//kythe/proto:analysis_cc_proto",
        "@io_kythe//kythe/proto:buildinfo_cc_proto",
    ],
)

cc_library(
    name = "comments",
    srcs = ["comments.cc"],
    hdrs = ["comments.h"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "comments_test",
    srcs = ["comments_test.cc"],
    deps = [
        ":comments",
        "@io_kythe//third_party:gtest_main",
    ],
)
