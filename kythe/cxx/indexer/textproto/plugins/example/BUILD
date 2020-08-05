load("//kythe/cxx/indexer/textproto/testdata:textproto_verifier_test.bzl", "textproto_verifier_test")

package(default_visibility = ["//visibility:public"])  # TODO

cc_library(
    name = "plugin",
    srcs = ["plugin.cc"],
    hdrs = ["plugin.h"],
    deps = [
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/indexer/proto:vname_util",
        "//kythe/cxx/indexer/textproto:plugin",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

textproto_verifier_test(
    name = "plugin_test",
    protos = ["example.proto"],
    textproto = "plugin_test.pbtxt",
)
