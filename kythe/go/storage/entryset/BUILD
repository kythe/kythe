load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "entryset",
    srcs = ["entryset.go"],
    importpath = "kythe.io/kythe/go/storage/entryset",
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/proto:entryset_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_pkg_errors//:errors",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "entryset_test",
    size = "small",
    srcs = ["entryset_test.go"],
    library = ":entryset",
    deps = [
        "//kythe/go/util/compare",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/proto:entryset_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/prototext",
        "@org_golang_google_protobuf//proto",
    ],
)
