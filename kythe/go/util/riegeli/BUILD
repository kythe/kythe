load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "riegeli",
    srcs = [
        "reader.go",
        "riegeli.go",
        "util.go",
        "writer.go",
    ],
    deps = [
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_minio_highwayhash//:go_default_library",
    ],
)

go_test(
    name = "riegeli_test",
    srcs = ["riegeli_test.go"],
    library = ":riegeli",
)

go_test(
    name = "golden_test",
    srcs = ["golden_test.go"],
    data = [
        "testdata/golden.entries.json",
        "testdata/golden.entries.riegeli",
    ],
    library = ":riegeli",
    deps = [
        "//kythe/go/storage/stream",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "block_test",
    srcs = ["block_test.go"],
    library = ":riegeli",
)

go_test(
    name = "riegeli_bench_test",
    srcs = ["riegeli_bench.go"],
    library = ":riegeli",
)
