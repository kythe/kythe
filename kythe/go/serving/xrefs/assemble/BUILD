load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "assemble",
    srcs = ["assemble.go"],
    importpath = "kythe.io/kythe/go/serving/xrefs/assemble",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/stream",
        "//kythe/go/util/compare",
        "//kythe/go/util/encoding/text",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/pager",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/go/util/schema/tickets",
        "//kythe/go/util/span",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "assemble_test",
    size = "small",
    srcs = ["assemble_test.go"],
    library = "assemble",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
