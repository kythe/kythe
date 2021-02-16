load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "beamio",
    srcs = [
        "beamio.go",
        "entries.go",
        "leveldb.go",
        "shards.go",
    ],
    deps = [
        "//kythe/go/storage/stream",
        "//kythe/go/util/riegeli",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/util/reflectx:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/io/filesystem:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/stats:go_default_library",
        "@com_github_syndtr_goleveldb//leveldb/comparer:go_default_library",
        "@com_github_syndtr_goleveldb//leveldb/journal:go_default_library",
        "@com_github_syndtr_goleveldb//leveldb/opt:go_default_library",
        "@com_github_syndtr_goleveldb//leveldb/table:go_default_library",
    ],
)

go_test(
    name = "entries_test",
    srcs = ["entries_test.go"],
    library = "beamio",
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/compare",
        "@com_github_apache_beam//sdks/go/pkg/beam/io/filesystem/local:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest:go_default_library",
    ],
)

go_test(
    name = "leveldb_test",
    srcs = ["leveldb_test.go"],
    library = "beamio",
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam/io/filesystem/local:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest:go_default_library",
        "@com_github_jmhodges_levigo//:go_default_library",
    ],
)

go_test(
    name = "shards_test",
    srcs = ["shards_test.go"],
    library = "beamio",
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert:go_default_library",
    ],
)
