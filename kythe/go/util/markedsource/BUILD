load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "markedsource",
    srcs = [
        "markedsource.go",
        "resolve.go",
    ],
    importpath = "kythe.io/kythe/go/util/markedsource",
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/md",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_html2markdown//escape",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "markedsource_test",
    size = "small",
    srcs = [
        "markedsource_test.go",
        "resolve_test.go",
    ],
    data = ["//kythe/cxx/doc"],
    library = ":markedsource",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/prototext",
        "@org_golang_google_protobuf//proto",
    ],
)
