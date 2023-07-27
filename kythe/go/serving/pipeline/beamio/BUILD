# gazelle:go_test file
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
    importpath = "kythe.io/kythe/go/serving/pipeline/beamio",
    deps = [
        "//kythe/go/storage/stream",
        "//kythe/go/util/log",
        "//kythe/go/util/riegeli",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/util/reflectx",
        "@com_github_apache_beam//sdks/go/pkg/beam/io/filesystem",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/stats",
        "@com_github_syndtr_goleveldb//leveldb/comparer",
        "@com_github_syndtr_goleveldb//leveldb/journal",
        "@com_github_syndtr_goleveldb//leveldb/opt",
        "@com_github_syndtr_goleveldb//leveldb/table",
    ],
)

go_test(
    name = "entries_test",
    srcs = ["entries_test.go"],
    library = "beamio",
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/compare",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/io/filesystem/local",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
    ],
)

go_test(
    name = "leveldb_test",
    srcs = ["leveldb_test.go"],
    library = "beamio",
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/io/filesystem/local",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/stats",
        "@com_github_jmhodges_levigo//:levigo",
    ],
)

go_test(
    name = "shards_test",
    srcs = ["shards_test.go"],
    library = "beamio",
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/stats",
    ],
)
