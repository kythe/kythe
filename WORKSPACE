workspace(name = "io_kythe")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("//:version.bzl", "check_version")

# Check that the user has a version between our minimum supported version of
# Bazel and our maximum supported version of Bazel.
check_version("0.18", "0.19")

load("//:setup.bzl", "kythe_rule_repositories")

kythe_rule_repositories()

load("//:external.bzl", "kythe_dependencies")

kythe_dependencies()

load("//tools/cpp:clang_configure.bzl", "clang_configure")

clang_configure()

http_archive(
    name = "bazel_toolchains",
    sha256 = "529f6763716f91e5b62bd14eeb5389b376126ecb276b127a78c95f8721280c11",
    strip_prefix = "bazel-toolchains-f95842b60173ce5f931ae7341488b6cb2610fd94",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/f95842b60173ce5f931ae7341488b6cb2610fd94.tar.gz",
        "https://github.com/bazelbuild/bazel-toolchains/archive/f95842b60173ce5f931ae7341488b6cb2610fd94.tar.gz",
    ],
)

bind(
    name = "libuuid",
    actual = "//third_party:libuuid",
)

bind(
    name = "libmemcached",
    actual = "@org_libmemcached_libmemcached//:libmemcached",
)

bind(
    name = "guava",  # required by @com_google_protobuf
    actual = "//third_party/guava",
)

bind(
    name = "gson",  # required by @com_google_protobuf
    actual = "@com_google_code_gson_gson//jar",
)

bind(
    name = "zlib",  # required by @com_google_protobuf
    actual = "@net_zlib//:zlib",
)

# Kept for third_party license
# TODO(schroederc): override bazel_rules_go dep once
#                   https://github.com/bazelbuild/rules_go/issues/1533 is fixed
new_git_repository(
    name = "go_protobuf",
    build_file = "@io_kythe//third_party/go:protobuf.BUILD",
    commit = "b4deda0973fb4c70b50d226b1af49f3da59f5265",
    remote = "https://github.com/golang/protobuf.git",
)

load("//tools/build_rules/external_tools:external_tools_configure.bzl", "external_tools_configure")

external_tools_configure()
