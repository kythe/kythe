package(
    features = ["layering_check"],
)

cc_binary(
    name = "proto_extractor",
    srcs = ["proto_extractor_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":lib",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:kzip_writer",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/proto:analysis_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "lib",
    srcs = ["proto_extractor.cc"],
    hdrs = ["proto_extractor.h"],
    visibility = ["//kythe/cxx/extractor/textproto:__subpackages__"],
    deps = [
        "//kythe/cxx/common:file_utils",
        "//kythe/cxx/common:index_writer",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/proto:analysis_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)
