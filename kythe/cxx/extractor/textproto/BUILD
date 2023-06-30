package(features = ["layering_check"])

cc_binary(
    name = "textproto_extractor",
    srcs = ["textproto_extractor.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":textproto_schema",
        "//kythe/cxx/common:file_utils",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:kzip_writer",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/extractor/proto:lib",
        "//kythe/cxx/indexer/proto:search_path",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "textproto_schema",
    srcs = ["textproto_schema.cc"],
    hdrs = ["textproto_schema.h"],
    visibility = ["//kythe/cxx/indexer/textproto:__pkg__"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "textproto_schema_test",
    srcs = ["textproto_schema_test.cc"],
    deps = [
        ":textproto_schema",
        "//third_party:gtest",
        "//third_party:gtest_main",
    ],
)
