workspace(name = "io_kythe")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("//:version.bzl", "check_version")

# Check that the user has a version between our minimum supported version of
# Bazel and our maximum supported version of Bazel, and not one of the known-bad releases.
check_version("0.25.1", "0.26")

http_archive(
    name = "bazel_toolchains",
    sha256 = "56e75f7c9bb074f35b71a9950917fbd036bd1433f9f5be7c04bace0e68eb804a",
    strip_prefix = "bazel-toolchains-9bd2748ec99d72bec41c88eecc3b7bd19d91a0c7",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/9bd2748ec99d72bec41c88eecc3b7bd19d91a0c7.tar.gz",
        "https://github.com/bazelbuild/bazel-toolchains/archive/9bd2748ec99d72bec41c88eecc3b7bd19d91a0c7.tar.gz",
    ],
)

load("//:setup.bzl", "kythe_rule_repositories", "maybe")

kythe_rule_repositories()

# TODO(schroederc): remove this.  This needs to be loaded before loading the
# go_* rules.  Normally, this is done by go_rules_dependencies in external.bzl,
# but because we want to overload some of those dependencies, we need the go_*
# rules before go_rules_dependencies.  Likewise, we can't precisely control
# when loads occur within a Starlark file so we now need to load this
# manually... https://github.com/bazelbuild/rules_go/issues/1966
load("@io_bazel_rules_go//go/private:compat/compat_repo.bzl", "go_rules_compat")

maybe(
    go_rules_compat,
    name = "io_bazel_rules_go_compat",
)

load("//:external.bzl", "kythe_dependencies")

kythe_dependencies()

load("//tools/build_rules/external_tools:external_tools_configure.bzl", "external_tools_configure")

external_tools_configure()

load("@build_bazel_rules_nodejs//:defs.bzl", "npm_install")
load("@build_bazel_rules_nodejs//:defs.bzl", "node_repositories")

node_repositories(package_json = ["//:package.json"])

npm_install(
    name = "npm",
    package_json = "//:package.json",
    package_lock_json = "//:package-lock.json",
)

# This binding is needed for protobuf. See https://github.com/protocolbuffers/protobuf/pull/5811
bind(
    name = "error_prone_annotations",
    actual = "@com_google_errorprone_error_prone_annotations//jar:jar",
)
