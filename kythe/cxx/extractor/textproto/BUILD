cc_binary(
    name = "textproto_extractor",
    srcs = ["textproto_extractor.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":textproto_schema",
        "//kythe/cxx/extractor/proto:lib",
        "//kythe/cxx/indexer/proto:search_path",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "textproto_schema",
    srcs = ["textproto_schema.cc"],
    hdrs = ["textproto_schema.h"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "textproto_schema_test",
    srcs = ["textproto_schema_test.cc"],
    deps = [
        ":textproto_schema",
        "@io_kythe//third_party:gtest_main",
    ],
)
