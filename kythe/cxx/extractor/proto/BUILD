package(features = ["layering_check"])

cc_binary(
    name = "proto_extractor",
    srcs = ["proto_extractor_main.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":lib",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:kzip_writer",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "lib",
    srcs = ["proto_extractor.cc"],
    hdrs = ["proto_extractor.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = ["//kythe/cxx/extractor/textproto:__subpackages__"],
    deps = [
        "//kythe/cxx/common:file_utils",
        "//kythe/cxx/common:file_vname_generator",
        "//kythe/cxx/common:index_writer",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/compiler:importer",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)
