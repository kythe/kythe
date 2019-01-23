cc_binary(
    name = "proto_extractor",
    srcs = ["proto_extractor_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":lib",
        "//kythe/cxx/indexer/proto:search_path",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:kzip_writer",
        "@io_kythe//kythe/cxx/common:path_utils",
        "@io_kythe//kythe/proto:analysis_cc_proto",
    ],
)

cc_library(
    name = "lib",
    srcs = ["proto_extractor.cc"],
    hdrs = ["proto_extractor.h"],
    visibility = ["//kythe/cxx/extractor/textproto:__subpackages__"],
    deps = [
        "//kythe/cxx/indexer/proto:search_path",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@io_kythe//kythe/cxx/common:index_writer",
        "@io_kythe//kythe/cxx/common:lib",
        "@io_kythe//kythe/cxx/common:path_utils",
        "@io_kythe//kythe/proto:analysis_cc_proto",
    ],
)
