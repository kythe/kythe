package(features = ["layering_check"])

cc_library(
    name = "proto_graph_builder",
    srcs = ["proto_graph_builder.cc"],
    hdrs = ["proto_graph_builder.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":comments",
        ":vname_util",
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:vname_ordering",
        "//kythe/cxx/common/indexing:output",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//kythe/proto:xref_cc_proto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":offset_util",
        ":proto_graph_builder",
        ":search_path",
        ":source_tree",
        "//kythe/cxx/common:file_vname_generator",
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/common:utf8_line_index",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/common/schema:edges",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:generated_message_info_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//kythe/proto:xref_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:node_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "vname_util",
    hdrs = ["vname_util.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/cxx/common:lib",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "source_tree",
    srcs = ["source_tree.cc"],
    hdrs = ["source_tree.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [
        "//kythe/cxx/indexer/textproto:__subpackages__",
    ],
    deps = [
        "//kythe/cxx/common:path_utils",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/compiler:importer",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)

cc_library(
    name = "search_path",
    srcs = ["search_path.cc"],
    hdrs = ["search_path.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [
        "//kythe/cxx/extractor/proto:__subpackages__",
        "//kythe/cxx/extractor/textproto:__subpackages__",
        "//kythe/cxx/indexer/textproto:__subpackages__",
    ],
    deps = [
        "//kythe/cxx/common:path_utils",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "indexer",
    srcs = ["kythe_indexer_main.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":proto_analyzer",
        "//kythe/cxx/common:file_vname_generator",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:kzip_reader",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/common/indexing:caching_output",
        "//kythe/cxx/common/indexing:output",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_google_protobuf//src/google/protobuf/io:gzip_stream",
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
        "//third_party:gtest",
        "//third_party:gtest_main",
    ],
)

cc_library(
    name = "offset_util",
    srcs = ["offset_util.cc"],
    hdrs = ["offset_util.h"],
    visibility = [
        "//kythe/cxx/indexer/textproto:__subpackages__",
    ],
    deps = [
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings",
    ],
)
