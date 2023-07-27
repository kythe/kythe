load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kythe",
    srcs = ["units.go"],
    importpath = "kythe.io/kythe/go/platform/kcd/kythe",
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "kythe_test",
    size = "small",
    srcs = ["units_test.go"],
    data = [
        "//kythe/testdata/platform:56bf5044e1b5c4c1cc7c4b131ac2fb979d288460e63352b10eef80ca35bd0a7b.pbtxt",
        "//kythe/testdata/platform:bb761979683e7c268e967eb5bcdedaa7fa5d1d472b0826b00b69acafbaad7ee6.pbtxt",
        "//kythe/testdata/platform:e9e170dcfca53c8126755bbc8b703994dedd3af32584291e01fba164ab5d3f32.pbtxt",
    ],
    library = ":kythe",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//ptypes/any",
        "@org_golang_google_protobuf//encoding/prototext",
    ],
)
