load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "viewindex",
    srcs = ["viewindex.go"],
    deprecation = "Please use `kzip view` instead",
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/kzip",
        "//kythe/go/util/flagutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
        "@org_golang_google_protobuf//encoding/protojson:go_default_library",
    ],
)
