# gazelle:go_test file
load("@rules_proto//proto:defs.bzl", "proto_library")
load("//kythe/proto:go.bzl", "go_proto_library")
load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "riegeli",
    srcs = [
        "compression.go",
        "reader.go",
        "riegeli.go",
        "transpose_decoder.go",
        "transpose_encoder.go",
        "transpose_util.go",
        "util.go",
        "writer.go",
    ],
    importpath = "kythe.io/kythe/go/util/riegeli",
    deps = [
        "//third_party/riegeli:records_metadata_go_proto",
        "@com_github_datadog_zstd//:zstd",
        "@com_github_golang_snappy//:snappy",
        "@com_github_google_brotli_go_cbrotli//:cbrotli",
        "@com_github_minio_highwayhash//:highwayhash",
        "@org_golang_google_protobuf//encoding/protowire",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "riegeli_test",
    srcs = ["riegeli_test.go"],
    library = ":riegeli",
    shard_count = 10,
    deps = [
        ":riegeli_test_go_proto",
        "//kythe/go/util/compare",
        "//kythe/go/util/log",
        "//third_party/riegeli:records_metadata_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "golden_test",
    srcs = ["golden_test.go"],
    data = [
        "testdata/golden.entries.brotli.riegeli",
        "testdata/golden.entries.brotli_transpose.riegeli",
        "testdata/golden.entries.json",
        "testdata/golden.entries.snappy.riegeli",
        "testdata/golden.entries.uncompressed.riegeli",
        "testdata/golden.entries.uncompressed_transpose.riegeli",
        "testdata/golden.entries.zstd.riegeli",
        "testdata/golden.records_metadata.textproto",
    ],
    deps = [
        ":riegeli",
        "//kythe/go/storage/stream",
        "//kythe/go/util/compare",
        "//kythe/proto:storage_go_proto",
        "//third_party/riegeli:records_metadata_go_proto",
        "@org_golang_google_protobuf//encoding/prototext",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "block_test",
    srcs = ["block_test.go"],
    library = ":riegeli",
)

go_test(
    name = "riegeli_bench_test",
    srcs = ["riegeli_bench_test.go"],
    library = ":riegeli",
)

go_test(
    name = "transpose_test",
    srcs = ["transpose_test.go"],
    library = ":riegeli",
)

proto_library(
    name = "riegeli_test_proto",
    srcs = ["riegeli_test.proto"],
)

go_proto_library(
    name = "riegeli_test_go_proto",
    importpath = "kythe.io/kythe/go/util/riegeli/riegeli_test_go_proto",
    proto = ":riegeli_test_proto",
    suggested_update_target = None,
)
