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
    sha256 = "4ab012a06e80172b1d2cc68a69f12237ba2c4eb47ba34cb8099830d3b8c43dbc",
    strip_prefix = "bazel-toolchains-646207624ed58c9dc658a135e40e578f8bbabf64",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/646207624ed58c9dc658a135e40e578f8bbabf64.tar.gz",
        "https://github.com/bazelbuild/bazel-toolchains/archive/646207624ed58c9dc658a135e40e578f8bbabf64.tar.gz",
    ],
)

load("//tools/build_rules/external_tools:external_tools_configure.bzl", "external_tools_configure")

external_tools_configure()
